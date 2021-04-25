use std::net::{TcpStream};
use std::str;
use std::io::{Write};
use std::time::Duration;
use std::io::{Read, Error};
use std::thread;
use simple_user_input::get_input;

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

//Can run it with cargo run --bin FTPClient
fn main() -> Result<(), Error>{

    let argv: Vec<_> = std::env::args().collect();
    let port = &argv[1];
    let mut path = "127.0.0.1:".to_owned();
    path.push_str(port);
    //do the connection to localhost with the given port
	let mut stream = TcpStream::connect(path).expect("aaa");
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
   

    while(cmd != "quit"){
        let mut buffer  = [0; 4096];
        println!("-------------------");
        cmd = get_input("insert command: ").to_string();
        let mut info:Vec<&str>= cmd.split(' ').collect();

//se hace el LIST
        if (info[0] == "ls"){
            let mut cmdSend :String = "LIST".to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); //Para que no se mande como un solo mensaje
            stream.write(cmdSend.as_bytes()).expect("Failed to write");
            println!("Server Data Response: \n");
            loop{
                
                let _ = stream.read(&mut buffer)?;
                thread::sleep(Duration::from_millis(100)); //Para que no se mande como un solo mensaje
                let mut data1 = str::from_utf8(&buffer).unwrap();
                println!("{}", data1);
                let tokens:Vec<&str>= data1.lines().collect();
                if check_exit(tokens){
                    break;
                }
                let clean_buffer: [u8; 4096] = [0; 4096];
                buffer = clean_buffer;     
            }
            let cleaner :String = "".to_string();
            cmdSend = cleaner;
            info.clear();
            
            
        }     
//se hace el GET 
        else if info[0] == "get"{
            
            let mut dataCmd = &info[1];
            let mut info = "RETR ".to_owned();
            info.push_str(dataCmd);
            let mut cleanInfo :String = info.to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); //Para que no se mande como un solo mensaje
            stream.write(cleanInfo.as_bytes()).expect("Failed to write");
            loop{
                let _ = stream.read(&mut buffer)?;
                thread::sleep(Duration::from_millis(100)); //Para que no se mande como un solo mensaje
                let mut data1 = str::from_utf8(&buffer).unwrap();
                println!("File response from server: {}", data1);
                let tokens:Vec<&str>= data1.lines().collect();
                if check_exit(tokens){
                    break;
                }
                let clean_buffer: [u8; 4096] = [0; 4096];
                buffer = clean_buffer;     
            }
            let cleaner :String = "".to_string();
            cleanInfo = cleaner;
            info.clear();
            
         
        }
//se hace el PUT
        else if info[0] == "put"{   
            let mut dataCmd1 = &info[1];
            let mut info = "STOR ".to_owned();
            info.push_str(dataCmd1);
            info.to_string();
            let mut cleanInfo2 :String = info.to_string();
            stream.write(client_data.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); //Para que no se mande como un solo mensaje
            stream.write(cleanInfo2.as_bytes()).expect("Failed to write");
            loop{
                let _ = stream.read(&mut buffer)?;
                thread::sleep(Duration::from_millis(100)); //Para que no se mande como un solo mensaje
                let mut data1 = str::from_utf8(&buffer).unwrap();
                println!("{}", data1);
                let tokens:Vec<&str>= data1.lines().collect();
                if check_exit(tokens){
                    break;
                }
                let clean_buffer: [u8; 4096] = [0; 4096];
                buffer = clean_buffer;     
            }
            let valid : String = "200".to_string();
            thread::sleep(Duration::from_millis(200)); //Para que no se mande como un solo mensaje
            stream.write(valid.as_bytes()).expect("Failed to write");
            let cleaner :String = "".to_string();
            cleanInfo2 = cleaner;
            info.clear();
            
            
        }
//se hace el QUIT
        else if info[0] == "quit"{
            let cmdSend2 : String = "QUIT".to_string();
            stream.write(cmdSend2.as_bytes()).expect("Failed to write");
            thread::sleep(Duration::from_millis(200)); //Para que no se mande como un solo mensaje
            println!("antes de read");
            let _ = stream.read(&mut buffer)?;
            //println!("despues de read");
            let data2 = str::from_utf8(&buffer).unwrap();
            println!("data: {}", data2);
            let tokens:Vec<&str>= data2.lines().collect();
            if check_exit(tokens){
                break;
            }
            let clean_buffer: [u8; 4096] = [0; 4096];
            buffer = clean_buffer;   
            info.clear();
            
            break;
        }

    
    }
    Ok(())

    
}

fn check_exit(tokens:Vec<&str>) ->bool{
    let mut check: bool = false;
    
    for s in tokens {
        let recvline:Vec<&str>= s.split(' ').collect();
        //println!("{:?}",recvline);  
        if recvline[0].contains("200"){ 
            check = true;
        }
        else if recvline[0].contains("221"){ 
            check = true;
        }
    }
    return check;
}

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