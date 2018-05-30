#! /usr/bin/env expect
set host [lindex $argv 0]
set passw [lindex $argv 1]
spawn ssh $host  pwd
expect {
    "assword:" {
        send "$passw\r"
        expect "home"
    }
    "home" {return}
}

