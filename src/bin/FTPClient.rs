use std::net::{TcpStream};
use std::str;
use std::io::{Write};
use std::time::Duration;
use std::io::{Read, Error};
use std::thread;
use simple_user_input::get_input;
use std::process;
use std::fs;

mod simple_user_input {
    use std::io;
    pub fn get_input(prompt: &str) -> String{
        println!("{}",prompt);
        let mut input = String::new();
        match io::stdin().read_line(&mut input) {
            Ok(_goes_into_input_above) => {},
            Err(_no_updates_is_fine) => {},
        }
        input.trim().to_string()
    }
}

//Can run it with cargo run --bin FTPClient .h 8080
fn main() -> Result<(), Error>{


    let argv: Vec<_> = std::env::args().collect();
    if !(argv.len()>=3 && argv.len() <5){        
        println!("Invalid Number of Arguments...");
        println!("Usage: ./ftpclient .h <host_port> [command]");
        process::exit(-1);
    }
    let mut newvec = argv.to_vec();
    let port = &argv[2];
    let mut path = "127.0.0.1:".to_owned();
    path.push_str(port);
    //do the connection to localhost with the given port
	let mut stream = TcpStream::connect(path).expect("Couldn't connect to server");
    println!("connection from {} to {}",
    stream.peer_addr().unwrap(),
    stream.local_addr().unwrap());

    let client_socket = stream.local_addr().unwrap().port() as i32;
    //convert the port so that we can have the 2 ports that the server needs
    let data : [i32;2] = convert(client_socket);
    let n5 : String = data[0].to_string();
    let n6 : String = data[1].to_string();
    let mut client_data: String = String::from("PORT 127,0,0,1,");
    client_data.push_str(&n5);
    client_data.push_str(",");
    client_data.push_str(&n6);
    println!("n5: {}, n6: {}", data[0], data[1]);
    println!("{}", client_data);
    //send message to the server with the client data
    
    
    
    let mut cmd: String = "".to_string();
   
    //we need a loop to keep asking for commands until the user disconnects from the server
    while(cmd != "quit"){
        let mut buffer  = [0; 16392];
        println!("-------------------");
        if argv.len() == 4 {
            cmd = newvec[3].to_string();
            thread::sleep(Duration::from_millis(2000));
        }else{        
            cmd = get_input("insert command: ").to_string();
        }
        let mut info:Vec<&str>= cmd.split(' ').collect();

        //If the client execute command LIST
        if (info[0] == "ls"){
            let mut cmdSend :String = "LIST".to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200));
            stream.write(cmdSend.as_bytes()).expect("Failed to write");
            println!("Server Data Response: \n");
            //a loop is needed to read all the data that the server sends back
            loop{
                
                let _ = stream.read(&mut buffer)?;
                thread::sleep(Duration::from_millis(100)); 
                let mut data1 = str::from_utf8(&buffer).unwrap();
                println!("{}", data1);
                let tokens:Vec<&str>= data1.lines().collect();
                if check_exit(tokens){
                    break;
                }
                let clean_buffer: [u8; 16392] = [0; 16392];
                buffer = clean_buffer;     
            }
            let cleaner :String = "".to_string();
            cmdSend = cleaner;
            info.clear();
            
            
        }     
        //If the client execute command GET 
        else if info[0] == "get"{
            
            let mut dataCmd = &info[1];
            let mut info = "RETR ".to_owned();
            info.push_str(dataCmd);
            let mut cleanInfo :String = info.to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); 
            stream.write(cleanInfo.as_bytes()).expect("Failed to write");
            let mut archive :String = "".to_owned();
            //a loop is needed to read all the data that the server sends back
            loop{
                let _ = stream.read(&mut buffer)?;
                thread::sleep(Duration::from_millis(100)); 
                let mut data1 = str::from_utf8(&buffer).unwrap();
                println!("File response from server: {}", data1);
                archive = archive + data1;
                let tokens:Vec<&str>= data1.lines().collect();
                if check_exit(tokens){
                    break;
                }
                let clean_buffer: [u8; 16392] = [0; 16392];
                buffer = clean_buffer;     
            }

            let tokens:Vec<&str>= archive.lines().collect();
            if !(check_error(tokens)){
                println!("guarda archivo");
                fs::write(dataCmd,archive);
            }
            
            let cleaner :String = "".to_string();
            cleanInfo = cleaner;
            info.clear();
        }
        //If the client execute command CD 
        else if info[0] == "cd"{
            let mut dataCmd = &info[1];
            let mut info = "CD ".to_owned();
            info.push_str(dataCmd);
            let mut cleanInfo :String = info.to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); 
            stream.write(cleanInfo.as_bytes()).expect("Failed to write");
            //a loop is needed to read all the data that the server sends back
            loop{
                let _ = stream.read(&mut buffer)?;
                thread::sleep(Duration::from_millis(100)); 
                let mut data1 = str::from_utf8(&buffer).unwrap();
                println!("File response from server: {}", data1);
                let tokens:Vec<&str>= data1.lines().collect();
                if check_exit(tokens){
                    break;
                }
                let clean_buffer: [u8; 16392] = [0; 16392];
                buffer = clean_buffer;     
            }
            let cleaner :String = "".to_string();
            cleanInfo = cleaner;
            info.clear();
        }
        //If the client execute command PWD 
        else if info[0] == "pwd"{
            let mut info = "PWD".to_owned();
            let mut cleanInfo :String = info.to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); 
            stream.write(cleanInfo.as_bytes()).expect("Failed to write");

            let _ = stream.read(&mut buffer)?;
            let mut data1 = str::from_utf8(&buffer).unwrap();
            println!("File response from server: {}", data1);
            let clean_buffer: [u8; 16392] = [0; 16392];
            buffer = clean_buffer;     
            
            let cleaner :String = "".to_string();
            cleanInfo = cleaner;
            info.clear();
        }
        //If the client execute command PUT
        else if info[0] == "put"{   
            let mut dataCmd1 = &info[1];
            let mut info = "STOR ".to_owned();
            info.push_str(dataCmd1);
            info.to_string();
            let mut cleanInfo2 :String = info.to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); 
            stream.write(cleanInfo2.as_bytes()).expect("Failed to write");
            let archiveInfo = fs::read_to_string(dataCmd1).expect("Unable to read file");
            println!("{}", archiveInfo);
            thread::sleep(Duration::from_millis(200)); 
            stream.write(archiveInfo.as_bytes()).expect("Failed to write");

            let valid : String = "200".to_string();
            thread::sleep(Duration::from_millis(200)); 
            stream.write(valid.as_bytes()).expect("Failed to write");
            let cleaner :String = "".to_string();
            cleanInfo2 = cleaner;
            info.clear();
            
            
        }
        //If the client execute command QUIT
        else if info[0] == "quit"{
            let cmdSend2 : String = "QUIT".to_string();
            stream.write(cmdSend2.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); 
            println!("antes de read");
            let _ = stream.read(&mut buffer)?;
            let data2 = str::from_utf8(&buffer).unwrap();
            println!("data: {}", data2);
            let tokens:Vec<&str>= data2.lines().collect();
            if check_exit(tokens){
                break;
            }
            let clean_buffer: [u8; 16392] = [0; 16392];
            buffer = clean_buffer;   
            info.clear();
            
            break;
        }

    
    }
    Ok(())

    
}
//we neec a function to catch every error that the server can send
fn check_error(tokens:Vec<&str>) ->bool{
    let mut check: bool = false;
    
    for s in tokens {
        let recvline:Vec<&str>= s.split(' ').collect();
        if recvline[0].contains("550") || recvline[0].contains("450") || recvline[0].contains("451"){ 
            check = true;
        }
    }
    return check;
}
//we need a function to catch the success of the execution of the command that the server respond.
fn check_exit(tokens:Vec<&str>) ->bool{
    let mut check: bool = false;
    
    for s in tokens {
        let recvline:Vec<&str>= s.split(' ').collect();
        if recvline[0].contains("200") || recvline[0].contains("450") || recvline[0].contains("451"){ 
            check = true;
        }
        else if recvline[0].contains("221") || recvline[0].contains("550"){ 
            check = true;
        }
    }
    return check;
}
//this function is needed so that the server knows what the clients port is
fn convert(mut port: i32) -> [i32;2]{
    let mut x = 1;
    let mut n5: i32 = 0;
    let mut n6: i32 = 0;
    let mut temp = 0;
    for i in 0..8{
        temp = port & x;
        n6 = (n6)|(temp);
        x = x << 1; 
    }
    port = port >> 8;
    x = 1;

    for i in 8..16{
        temp = port & x;
        n5 = ((n5)|(temp));
        x = x << 1; 
    }
	let test: [i32; 2] = [n5,n6];
    return test;
}