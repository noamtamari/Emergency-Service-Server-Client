package bgu.spl.net.impl.stomp;

import java.util.HashMap;

public class test {

    public static void main(String[] args) {
        testFrameCreation();
        testSetAndGetHeaders();
        testSetAndGetBody();
    }

    private static void testFrameCreation() {
        System.out.println("Running testFrameCreation...");
        Frame frame = new Frame("MESSAGE");
        if (!frame.getType().equals("MESSAGE")) {
            System.out.println("FAILED: Frame type is incorrect!");
        } else {
            System.out.println("PASSED: Frame type is correct.");
        }
    }

    private static void testSetAndGetHeaders() {
        System.out.println("Running testSetAndGetHeaders...");
        Frame frame = new Frame("CONNECT");
        frame.addHeader("host", "stomp.github.io");
        frame.addHeader("accept-version", "1.2");

        if (!frame.getHeader("host").equals("stomp.github.io")) {
            System.out.println("FAILED: Header 'host' is incorrect!");
        } else if (!frame.getHeader("accept-version").equals("1.2")) {
            System.out.println("FAILED: Header 'accept-version' is incorrect!");
        } else {
            System.out.println("PASSED: Headers are correct.");
        }
    }

    private static void testSetAndGetBody() {
        System.out.println("Running testSetAndGetBody...");
        Frame frame = new Frame("SEND");
        frame.setMessageBody("Hello, STOMP!");

        if (!frame.getMessageBody().equals("Hello, STOMP!")) {
            System.out.println("FAILED: Message body is incorrect!");
        } else {
            System.out.println("PASSED: Message body is correct.");
        }
    }
}
