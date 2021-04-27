
use std::process::Command;
use std::thread;
use std::time::Duration;
use std::process;



fn main() {
    //To run this program you need the following arguments
	//amountOfThreads, executable, ip, port, clientCommand

    //Example of how to run stressCMD
    //cargo run --bin StressCMD .n 6 ./ftpclient -h 8080 ls
    //cargo run --bin StressCMD .n 6 ./FTPClient .h 8080 ls IF RUST CLIENT
    let argv: Vec<std::string::String> = std::env::args().collect();
    if argv.len() <7{        
        println!("Invalid Number of Arguments...");
        println!("Usage: .n <number_threads> ftpclient <client_params>");
        process::exit(-1);
    }   
    let mut newvec = argv.to_vec();
    let mut newvec2 = argv.to_vec();

    let loops:i32 = argv[2].parse().unwrap();
    let mut i:i32 = 0;
    thread::spawn(move||{
        while(i < loops){
            let mut client_cmd = &mut newvec2[6];
            if argv.len() == 8 && (client_cmd == "cd" || client_cmd == "get" || client_cmd == "put"){
                client_cmd.push_str(" ");
                client_cmd.push_str(&argv[7]);
            }
            Command::new(&mut newvec[3]).arg(&mut newvec[4]).arg(&mut newvec[5]).arg(&client_cmd).spawn().unwrap();
            println!("hi number {} from the spawned thread!", i);
            thread::sleep(Duration::from_millis(1));
            i+=1;
        }
    });
    while(true){//I need an infinite loop, otherwise the main thread ends, and so will the other ones created.

    }

}