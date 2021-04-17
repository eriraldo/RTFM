
use std::process::Command;
//pa tarea 3
use std::thread;
use std::time::Duration;
use std::env;
//


fn main() {
    let argv: Vec<std::string::String> = std::env::args().collect();
    let mut newvec = argv.to_vec();
    let loops:i32 = argv[1].parse().unwrap();
    println!("{}", loops); 
    let mut i:i32 = 0;
    thread::spawn(move||{
        while(i < loops){
            Command::new(&mut newvec[2]).arg(&mut newvec[3]).arg(&mut newvec[4]).arg(&mut newvec[5]).spawn().unwrap();
            println!("hi number {} from the spawned thread!", i);
            thread::sleep(Duration::from_millis(1));
            i+=1;
        }
    });
    while(true){

    }

}