#! /usr/bin/env python3
import tarfile
import typing
from math import ceil

import numpy as np
import argparse
import json
from os.path import isfile, join, isdir

import tensorflow as tf

Modes = tf.estimator.ModeKeys


def check_file(value):
    if isfile(value):
        return value
    else:
        raise argparse.ArgumentTypeError("Not valid path to file")


def directory(value):
    if isdir(value):
        return value
    else:
        raise argparse.ArgumentTypeError("Not valid path to save dir")


class DynamicRead(object):
    def __init__(self, tar_object):
        self.tar = tar_object
        for file_info in self.tar.getmembers():
            if not file_info.isfile():
                continue
            self.val = self.tar.extractfile(file_info).__next__()
            break

    def __getitem__(self, item):
        return self.val

    def __iter__(self):
        for file_info in self.tar.getmembers():
            if not file_info.isfile():
                continue
            for line in self.tar.extractfile(file_info).readlines():
                yield line




def read_all_data(tar_path):
    tar = tarfile.open(tar_path)
    return DynamicRead(tar)

def read_data(tar_path, sample_per_file: int = 100000, split_factor: float = 0.8):
    """

    :return: 
    """
    tar = tarfile.open(tar_path)
    final_content = []
    for file_info in tar.getmembers():
        if not file_info.isfile():
            continue
        content = tar.extractfile(file_info).readlines()
        content2 = np.random.choice(content, sample_per_file)
        final_content.append(content2)
    tar.close()
    samples = np.concatenate(final_content)
    msk = np.random.rand(len(samples)) < split_factor

    return samples[msk], samples[~msk]


def find_column_separator_position(line):
    if isinstance(line, str):
        return line.split().index("|")
    return line.decode().split().index("|")


def marking_array(pos, length=9):
    res = [0] * length
    res[pos] = 1
    return res


def gen(content, separator_pos: int, value_to_class_map: typing.List[dict]):
    size_of_targets = max(map(len, value_to_class_map))
    def gen2():
        print("aaaaa")
        np.random.shuffle(content)
        for el in content:
            values = el.decode().split()
            yield {"inputs": [x if x < 1000 else 1 for x in map(int, values[:separator_pos])],
                   "targets":
                       [marking_array(d[v], size_of_targets) for d, v in zip(value_to_class_map, values[separator_pos + 1:])],
                   "results": [d[v] for d, v in zip(value_to_class_map, values[19:])]}

    return gen2

def gen_vithout_shuffle(content, separator_pos: int, value_to_class_map: typing.List[dict]):
    size_of_targets = max(map(len, value_to_class_map))
    def gen2():
        print("aaaaa")
        for el in content:
            values = el.decode().split()
            yield {"inputs": [x if x < 1000 else 1 for x in map(int, values[:separator_pos])],
                   "targets":
                       [marking_array(d[v], size_of_targets) for d, v in zip(value_to_class_map, values[separator_pos + 1:])],
                   "results": [d[v] for d, v in zip(value_to_class_map, values[19:])]}

    return gen2




class MetaboliteNN(object):
    def __init__(self, layer_size, layers_num):
        self.train_data = np.zeros((1))
        self.test_data = np.zeros((1))
        self.value_to_class_map = [{}]
        self.layer_size = layer_size
        self.layers_num = layers_num

    def set_size_variables(self):
        self.size_of_marking = find_column_separator_position(self.test_data[0])
        self.size_of_targets = max(map(len, self.value_to_class_map))
        self.number_of_targets = len(self.value_to_class_map)

    def read_data(self, path_to_tar, path_to_json):
        self.train_data, self.test_data = read_data(path_to_tar)
        with open(path_to_json) as ff:
            self.value_to_class_map = json.load(ff)
        self.set_size_variables()

    def dump_data(self):
        np.savez("sample.npz", self.train_data, self.test_data)

    def read_dump(self, path_to_json):
        sample = np.load("./sample.npz")
        self.train_data= sample["arr_0"]
        self.test_data  = sample["arr_1"]
        with open(path_to_json) as ff:
            self.value_to_class_map = json.load(ff)
        self.set_size_variables()


    def read_for_test(self, path_to_tar, path_to_json):
        self.test_data = read_all_data(path_to_tar)
        with open(path_to_json) as ff:
            self.value_to_class_map = json.load(ff)
        self.set_size_variables()

    def prepare_train(self):
        types = {"inputs": tf.float32, "targets": tf.float32, "results": tf.int64}
        shapes = {"inputs": tf.TensorShape([self.size_of_marking]),
                  "targets": tf.TensorShape([self.number_of_targets, self.size_of_targets]),
                  "results": tf.TensorShape(self.number_of_targets)}

        train_ds = tf.data.Dataset().from_generator(gen(self.train_data, self.size_of_marking, self.value_to_class_map),
                                                    output_types=types,
                                                    output_shapes=shapes)
        train_batches = train_ds.repeat().batch(BATCH_SIZE)
        self.train_batch = train_batches.make_one_shot_iterator().get_next()

    def prepare_test(self, only=False):
        types = {"inputs": tf.float32, "targets": tf.float32, "results": tf.int64}
        shapes = {"inputs": tf.TensorShape([self.size_of_marking]),
                  "targets": tf.TensorShape([self.number_of_targets, self.size_of_targets]),
                  "results": tf.TensorShape(self.number_of_targets)}
        if only:
            test_ds = tf.data.Dataset().from_generator(gen_vithout_shuffle(self.test_data, self.size_of_marking, self.value_to_class_map),
                                                   output_types=types,
                                                   output_shapes=shapes)
        else:
            test_ds = tf.data.Dataset().from_generator(gen(self.test_data, self.size_of_marking, self.value_to_class_map),
                                                   output_types=types,
                                                   output_shapes=shapes)


        eval_batches = test_ds.repeat().batch(BATCH_SIZE)
        self.eval_batch = eval_batches.make_one_shot_iterator().get_next()

    def model(self, batch, mode, fully_connected=True):
        with tf.variable_scope("mymodel", reuse=mode == Modes.EVAL):
            # Inputs.
            x, y, z = batch["inputs"], batch["targets"], batch["results"]
            x = tf.reshape(x, [BATCH_SIZE, self.size_of_marking])  # Height and width on channels.
            y = tf.reshape(y, [BATCH_SIZE, self.number_of_targets, self.size_of_targets])  # Bogus dimension.
            z = tf.reshape(z, [BATCH_SIZE, self.number_of_targets])
            # Body.
            hidden_size = 128
            h = tf.layers.dense(x, self.layer_size, activation=tf.nn.relu, name="hidden")
            hh = [x, h]
            for i in range(self.layers_num - 1):
                if fully_connected:
                    prev = tf.concat(hh, axis=1)
                else:
                    prev = hh[-1]
                hh.append(tf.layers.dense(prev, hidden_size, activation=tf.nn.relu, name=f"hidden{i}"))
            o1 = tf.layers.dense(hh[-1], 126, name="output")
            o = tf.reshape(o1, [BATCH_SIZE, 14, 9])
            print(type(o))
            # Loss and accuracy.
            # ipdb.set_trace()
            loss = tf.nn.sigmoid_cross_entropy_with_logits(logits=o, labels=y)
            accuracy = tf.to_float(tf.equal(tf.argmax(o, axis=-1), z))

            return tf.reduce_mean(loss), tf.reduce_mean(accuracy), (accuracy, x, z, tf.argmax(o, axis=-1))

    def run(self, save_path: str, restart: bool, save_path2: typing.Union[str, None] = None,
            num_steps: int = 12000, fully_connected: bool =True):

        tf.reset_default_graph()
        with tf.Session() as sess:
            self.prepare_test()
            self.prepare_train()

            # Model for train.
            train_loss, train_accuracy, _ = self.model(self.train_batch, Modes.TRAIN, fully_connected)
            # Gradients.
            train_op = tf.train.AdamOptimizer().minimize(train_loss)
            # Model for eval.
            eval_loss, eval_accuracy, _ = self.model(self.eval_batch, Modes.EVAL, fully_connected)

            saver = tf.train.Saver()

            print("Start training")

            if restart:
                if save_path2 is not None:
                    saver.restore(sess, join(save_path2, "model.ckpt"))
                else:
                    saver.restore(sess, join(save_path, "model.ckpt"))

            else:
                sess.run(tf.global_variables_initializer())
            for step in range(num_steps + 1):
                _, loss, accuracy = sess.run([train_op, train_loss, train_accuracy])
                if step % 100 == 0:
                    print("Step %d train loss %.4f accuracy %.2f" % (step, loss, accuracy))
                    loss, accuracy = sess.run([eval_loss, eval_accuracy])
                    print("Step %d eval loss %.4f accuracy %.2f" % (step, loss, accuracy))

            save_path = saver.save(sess, join(save_path, "model.ckpt"))
            print(f"Model saved to {save_path}")

    def run_test(self, save_path: str, num_steps: int = 12000, fully_connected: bool = True):
        bad_examples = 0
        tf.reset_default_graph()
        with tf.Session() as sess:
            num_steps =  int(2*(3**17)  // BATCH_SIZE)
            print(f"Steps to check {num_steps}")
            self.prepare_test(True)

            eval_loss, eval_accuracy, (valid, x, z, out) = self.model(self.eval_batch, Modes.TRAIN, fully_connected)

            saver = tf.train.Saver()
            saver.restore(sess, join(save_path, "model.ckpt"))
            with open(join(save_path, "problematic.txt"), 'w') as ff:
                for step in range(num_steps):
                    loss, accuracy, valid_r, x_r, z_r, out_r = sess.run([eval_loss, eval_accuracy, valid, x, z, out])
                    # print(valid_r.shape)
                    mask = np.min(valid_r, axis=-1).astype(np.uint8) == 0
                    if np.any(mask):
                        for mark, res, res2 in zip(x_r.astype(np.uint32)[mask], z_r[mask], out_r[mask]):
                            for m in mark:
                                ff.write(f"{m} ")
                            ff.write("|")
                            for r in res:
                                ff.write(f" {r}")
                            ff.write(" |")
                            for r in res2:
                                ff.write(f" {r}")
                            ff.write("\n")

                    bad_examples += np.sum(mask)
                    if step % 100 == 0:
                        print(f"step: {step}, Bad examples_num: {bad_examples}")

            #save_path = saver.save(sess, join(save_path, "model.ckpt"))
            print(f"Bad examples_num: {bad_examples}")


def main():
    parser = argparse.ArgumentParser("Train Network")
    parser.add_argument("path_to_tar", help="Path to archive with simplex solutions", type=check_file)
    parser.add_argument("path_to_json", help="path to json file with metadata", type=check_file)
    parser.add_argument("path_to_save", help="path to save location")
    parser.add_argument('--restart', help="restart learning", nargs='?', default=False, const=True, type=directory)
    parser.add_argument("-i,--iterations", help="number of iterations", dest="iterations", type=int,
                        default=1200)
    parser.add_argument("-b,--batch_size", help="number of elements in batch", dest="batch_size", type=int,
                        default=10000)
    parser.add_argument("--no_fully_connect", action="store_true")
    parser.add_argument("--test", action="store_true")
    parser.add_argument("--layer_size", type=int, default=128)
    parser.add_argument("--layers_num", type=int, default=3, help="hidden layers number")
    args = parser.parse_args()
    global BATCH_SIZE
    print(args)
    BATCH_SIZE = args.batch_size
    nn = MetaboliteNN(args.layer_size, args.layers_num)
    if args.test:
        nn.read_for_test(args.path_to_tar, args.path_to_json)
        #nn.read_dump(args.path_to_json)
        nn.run_test(args.path_to_save, num_steps=args.iterations, fully_connected=not args.no_fully_connect)
        nn.test_data.tar.close()
    else:
        nn.read_data(args.path_to_tar, args.path_to_json)
        #nn.read_dump(args.path_to_json)
        nn.run(args.path_to_save, bool(args.restart), num_steps=args.iterations,
               save_path2=args.restart if isinstance(args.restart, str) else None,
               fully_connected=not args.no_fully_connect)

if __name__ == '__main__':
    main()
