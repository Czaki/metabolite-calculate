#! /usr/bin/env expect
set host [lindex $argv 0]
set passw [lindex $argv 1]
spawn ssh $host  ls sroka
expect {
    "assword:" {
        send "$passw\r"
        expect "metabolite-calculate"
    }
    "metabolite-calculate" {return}
}

