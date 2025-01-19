package bgu.spl.net.impl.stomp;

import java.nio.charset.StandardCharsets;

public class test {

    public static void main(String[] args) {

        Frame frame = new Frame("MESSAGE");
        frame.addHeader("destination", "/queue/test");
        frame.addHeader("subscription", "42");
        frame.setMessageBody("Hello, STOMP!");

        // Create the encoder/decoder
        StompEncoderDecoder encoderDecoder = new StompEncoderDecoder();

        // Encode the frame to bytes
        byte[] encodedBytes = encoderDecoder.encode(frame);

        // Print the encoded bytes as a string for verification
        System.out.println(new String(encodedBytes, StandardCharsets.UTF_8));

        // Simulate a received STOMP frame in bytes
        byte[] receivedBytes = ("MESSAGE\n" +
                                "destination:/queue/test\n" +
                                "subscription:42\n\n" +
                                "Hello, STOMP!\u0000").getBytes(StandardCharsets.UTF_8);

        Frame decodedFrame = null;

        for (byte b : receivedBytes) {
            decodedFrame = encoderDecoder.decodeNextByte(b);
            if (decodedFrame != null) {
                break; // Frame is fully decoded
            }
        }
        // Print the decoded frame's details
        System.out.println("Frame Type: " + decodedFrame.getType());
        System.out.println("Destination: " + decodedFrame.getHeader("destination"));
        System.out.println("Body: " + decodedFrame.getMessageBody());
    }
}
