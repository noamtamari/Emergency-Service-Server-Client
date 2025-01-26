package bgu.spl.net.impl.stomp;

import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;

public class Frame {
    private String type; // Frame type (e.g., CONNECT, SEND, SUBSCRIBE, etc.)
    private LinkedHashMap<String, String> headers; // Key-value pairs for headers
    private String messageBody; // Optional message body

    // Constructor
    public Frame(String type) {
        this.type = type;
        this.headers = new LinkedHashMap<>();
        this.messageBody = "\n";
    }

    public Frame(String type, String messageBody) {
        this.type = type;
        this.headers = new LinkedHashMap<>();
        this.messageBody = messageBody;
    }

    public Frame(Frame frame, String subscriptionId, int messageId, String channel) {
        this.type = frame.getType();
        this.headers = new LinkedHashMap<>();
        this.headers.put("subscription", subscriptionId);
        this.headers.put("message-id", String.valueOf(messageId));
        this.headers.put("destination", channel);
        this.messageBody = frame.getMessageBody();
    }

    // Add a header
    public void addHeader(String key, String value) {
        headers.put(key, value);
    }

    // Get a header by key
    public String getHeader(String key) {
        return headers.get(key);
    }

    // Set the message body
    public void setMessageBody(String messageBody) {
        this.messageBody = messageBody;
    }

    // Get the message body
    public String getMessageBody() {
        return messageBody;
    }

    // Get the frame type
    public String getType() {
        return type;
    }

    // Serialize the frame to a STOMP-formatted string
    @Override
    public String toString() {
        StringBuilder frameString = new StringBuilder();
        frameString.append(type).append("\n");

        // Append headers
        for (String key : headers.keySet()) {
            frameString.append(key).append(":").append(headers.get(key)).append("\n");
        }

        // End headers with a blank line
        frameString.append("\n");

        // Append the message body (if exists) and the null terminator
        if (!messageBody.isEmpty()) {
            frameString.append(messageBody);
        }
        frameString.append("\u0000"); // STOMP null terminator
        return frameString.toString();
    }

    public String stringMessage() {
        StringBuilder frameString = new StringBuilder();
        frameString.append(type).append("\n");

        // Append headers
        for (String key : headers.keySet()) {
            frameString.append(key).append(":").append(headers.get(key)).append("\n");
        }

        // Append the message body (if exists) and the null terminator
        if (!messageBody.isEmpty()) {
            frameString.append(messageBody);
        }
        frameString.append("\n");
        return frameString.toString();
    }

    // Static method to parse a STOMP frame from a byte array
    public static Frame fromBytes(byte[] rawBytes) {
        // Convert the byte array to a string using UTF-8 encoding
        String rawFrame = new String(rawBytes, StandardCharsets.UTF_8);

        // Split the message into lines
        String[] lines = rawFrame.split("\n");
        if (lines.length == 0) {
            throw new IllegalArgumentException("Invalid STOMP frame: No type found");
        }

        // The first line is the frame type
        String type = lines[0];
        Frame frame = new Frame(type);

        // Parse headers (until an empty line)
        int i = 1;
        while (i < lines.length && !lines[i].isEmpty()) {
            String[] headerParts = lines[i].split(":", 2);
            if (headerParts.length == 2) {
                frame.addHeader(headerParts[0], headerParts[1]);
            }
            i++;
        }

        // Parse message body (everything after the blank line until the null terminator)
        StringBuilder bodyBuilder = new StringBuilder();
        i++; // Skip the blank line
        while (i < lines.length) {
            String line = lines[i];
            if (line.equals("\u0000")) { // Stop at the null terminator
                break;
            }
            bodyBuilder.append(line).append("\n");
            i++;
        }

        // Remove the trailing newline character from the body
        if (bodyBuilder.length() > 0) {
            bodyBuilder.setLength(bodyBuilder.length() - 1);
        }
        frame.setMessageBody(bodyBuilder.toString());

        return frame;
    }

    /**
     * Converts the Frame object to a byte array.
     *
     * @return a byte array representing the serialized STOMP frame
     */
    public byte[] toBytes() {
        // Serialize the frame to a string and convert to bytes
        return this.toString().getBytes(StandardCharsets.UTF_8);
    }
}
