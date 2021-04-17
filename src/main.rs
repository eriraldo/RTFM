
use std::process::Command;
use std::thread;
use std::time::Duration;
use std::env;



fn main() {
    //To run this program you need the following arguments
	//amountOfThreads, executable, ip, port, clientCommand

    //Example of how to run stressCMD
    //cargo run 5 ./ftpclient 127.0.0.1 8080 ls
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
    while(true){//I need an infinite loop, otherwise the main thread ends, and so will the other ones created.

    }

}