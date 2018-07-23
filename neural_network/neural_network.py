#! /usr/bin/env python3
import tarfile
import typing

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


def read_data(tar_path, sample_per_file: int = 10000, split_factor: float = 0.8):
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
            yield {"inputs": list(map(int, values[:separator_pos])),
                   "targets":
                       [marking_array(d[v], size_of_targets) for d, v in zip(value_to_class_map, values[separator_pos + 1:])],
                   "results": [d[v] for d, v in zip(value_to_class_map, values[19:])]}

    return gen2


def model(batch, mode):
    with tf.variable_scope("mymodel", reuse=mode == Modes.EVAL):
        # Inputs.
        x, y, z = batch["inputs"], batch["targets"], batch["results"]
        x = tf.reshape(x, [BATCH_SIZE, size_of_marking])  # Height and width on channels.
        y = tf.reshape(y, [BATCH_SIZE, number_of_targets, size_of_targets])  # Bogus dimension.
        z = tf.reshape(z, [BATCH_SIZE, number_of_targets])
        # Body.
        hidden_size = 128
        h = tf.layers.dense(x, hidden_size, activation=tf.nn.relu, name="hidden")
        hh = [h]
        for i in range(2):
            hh.append(tf.layers.dense(hh[-1], hidden_size, activation=tf.nn.relu, name=f"hidden{i}"))
        o1 = tf.layers.dense(hh[-1], 126, name="output")
        o = tf.reshape(o1, [BATCH_SIZE, 14, 9])
        print(type(o))
        # Loss and accuracy.
        # ipdb.set_trace()
        loss = tf.nn.sigmoid_cross_entropy_with_logits(logits=o, labels=y)
        accuracy = tf.to_float(tf.equal(tf.argmax(o, axis=-1), z))

        return tf.reduce_mean(loss), tf.reduce_mean(accuracy)


def run(path_to_tar, path_to_json, save_path: str, restart: bool, save_path2: typing.Union[str, None] = None,
        num_steps: int = 12000):
    with open(path_to_json) as ff:
        value_to_class_map = json.load(ff)
    train_data, test_data = read_data(path_to_tar)
    # np.savez("sample.npz", train_data, test_data)
    #sample = np.load("./sample.npz")
    #train_data, test_data = sample["arr_0"], sample["arr_1"]
    global size_of_marking, size_of_targets, number_of_targets
    size_of_marking = find_column_separator_position(train_data[0])
    size_of_targets_second_dim = len(value_to_class_map)
    tf.reset_default_graph()
    sess = tf.Session()

    size_of_targets = max(map(len, value_to_class_map))
    number_of_targets = len(value_to_class_map)
    types = {"inputs": tf.float32, "targets": tf.float32, "results": tf.int64}
    shapes = {"inputs": tf.TensorShape([size_of_marking]), "targets": tf.TensorShape([number_of_targets, size_of_targets]),
              "results": tf.TensorShape(len(value_to_class_map))}

    train_ds = tf.data.Dataset().from_generator(gen(train_data, size_of_marking, value_to_class_map), output_types=types,
                                                output_shapes=shapes)
    test_ds = tf.data.Dataset().from_generator(gen(test_data, size_of_marking, value_to_class_map), output_types=types,
                                               output_shapes=shapes)

    train_batches = train_ds.repeat().batch(BATCH_SIZE)
    train_batch = train_batches.make_one_shot_iterator().get_next()
    eval_batches = test_ds.repeat().batch(BATCH_SIZE)
    eval_batch = eval_batches.make_one_shot_iterator().get_next()

    # Model for train.
    train_loss, train_accuracy = model(train_batch, Modes.TRAIN)
    # Gradients.
    train_op = tf.train.AdamOptimizer().minimize(train_loss)
    # Model for eval.
    eval_loss, eval_accuracy = model(eval_batch, Modes.EVAL)

    saver = tf.train.Saver()

    print("Start training")

    if restart:
        if save_path2 is not None:
            saver.restore(sess, join(save_path2, "model.ckpt"))
        else:
            saver.restore(sess, join(save_path, "model.ckpt"))

    else:
        sess.run(tf.global_variables_initializer())
        # 2 epochs on 60K examples.
    for step in range(num_steps + 1):
        _, loss, accuracy = sess.run([train_op, train_loss, train_accuracy])
        if step % 100 == 0:
            print("Step %d train loss %.4f accuracy %.2f" % (step, loss, accuracy))
            loss, accuracy = sess.run([eval_loss, eval_accuracy])
            print("Step %d eval loss %.4f accuracy %.2f" % (step, loss, accuracy))

    save_path = saver.save(sess, join(save_path, "model.ckpt"))
    print(f"Model saved to {save_path}")
    sess.close()


def main():
    parser = argparse.ArgumentParser("Train Network")
    parser.add_argument("path_to_tar", help="Path to archive with simplex solutions", type=check_file)
    parser.add_argument("path_to_json", help="path to json file with metadata", type=check_file)
    parser.add_argument("path_to_save", help="path to save location")
    parser.add_argument('--restart', help="restart learning", nargs='?', default=False, const=True, type=directory)
    parser.add_argument("-i,--iterations", help="number of iterations", dest="iterations", type=int,
                        default=1200)
    parser.add_argument("-b,--batch_size", help="number of elements in batch", dest="batch_size", type=int,
                        default=1000)
    args = parser.parse_args()
    global BATCH_SIZE
    BATCH_SIZE = args.batch_size
    print(args)
    run(args.path_to_tar, args.path_to_json, args.path_to_save, bool(args.restart), num_steps=args.iterations,
        save_path2=args.restart if isinstance(args.restart, str) else None)


if __name__ == '__main__':
    main()
