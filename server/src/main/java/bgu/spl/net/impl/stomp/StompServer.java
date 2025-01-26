package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {

        if (args.length < 2) {
            System.out.println("Usage: StompServer <port> <type>");
            return;
        }
        int port = Integer.parseInt(args[0]);
        String type = args[1];

        if (type.equals("tpc")) {
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
