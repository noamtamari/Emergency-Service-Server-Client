package bgu.spl.net.impl.stomp;

import java.util.Scanner;

import bgu.spl.net.impl.echo.LineMessageEncoderDecoder;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
      
        Scanner scanner = new Scanner(System.in);
        System.out.println("Enter port and type (e.g., 7777 tpc or reactor):");
        String input = scanner.nextLine();
        String[] inputArgs = input.split(" ");

        if (inputArgs.length < 2) {
            System.out.println("Usage: StompServer <port> <type>");
            return;
        }


        

        int port = Integer.parseInt(inputArgs[0]);
        String type = inputArgs[1];

        if (type.equals("tpc")){
            Server.threadPerClient(
                        port, // port
                        StompMessagingProtocolImpl::new, // protocol factory
                        StompEncoderDecoder::new // message encoder-decoder factory
            ).serve();
        }
        if (type.equals("reactor")) {
            // Reactor server
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(), // number of threads
                    port, // port
                    StompMessagingProtocolImpl::new, // protocol factory
                    StompEncoderDecoder::new // message encoder-decoder factory
            ).serve();
        } else {
            System.out.println("Illegal command");
        }
    }
}
