package bgu.spl.net.impl.stomp;

import bgu.spl.net.impl.echo.LineMessageEncoderDecoder;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("Usage: StompServer <port> <type>");
            return;
        }

        int port = Integer.parseInt(args[0]);
        String type = args[1];

        if ("tcp".equalsIgnoreCase(type)) {
            Server.threadPerClient(
                    port, // port
                    StompMessagingProtocolImpl::new, // protocol factory
                    StompEncoderDecoder::new // message encoder decoder factory
            ).serve();
        } else if ("reactor".equalsIgnoreCase(type)) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port, // port
                    StompMessagingProtocolImpl::new, // protocol factory
                    StompEncoderDecoder::new // message encoder decoder factory
            ).serve();
        } else {
            System.out.println("Illegal command");
        }
    }
}
