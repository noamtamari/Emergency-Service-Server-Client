package bgu.spl.net.impl.stomp;

import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;

/**
 * The Frame class represents a STOMP frame, which consists of a command,
 * headers, and a message body.
 */
public class Frame {
    private String type; // Frame type (e.g., CONNECT, SEND, SUBSCRIBE, etc.)
    private LinkedHashMap<String, String> headers; // Key-value pairs for headers
    private String messageBody; // Optional message body

    /**
     * Constructs a {@code Frame} with the specified type and initializes an empty
     * headers map.
     *
     * @param type the type of the STOMP frame (e.g., CONNECT, SEND, SUBSCRIBE)
     */
    public Frame(String type) {
        this.type = type;
        this.headers = new LinkedHashMap<>();
        this.messageBody = "\n";
    }

    /**
     * Constructs a {@code Frame} with the specified type and message body.
     *
     * @param type        the type of the STOMP frame
     * @param messageBody the message body of the frame
     */
    public Frame(String type, String messageBody) {
        this.type = type;
        this.headers = new LinkedHashMap<>();
        this.messageBody = messageBody;
    }

    /**
     * Constructs a {@code Frame} based on another frame, adding
     * subscription-specific headers.
     *
     * @param frame          the original frame to copy from
     * @param subscriptionId the subscription ID to add to the headers
     * @param messageId      the message ID to add to the headers
     * @param channel        the destination channel to add to the headers
     */
    public Frame(Frame frame, String subscriptionId, int messageId, String channel) {
        this.type = frame.getType();
        this.headers = new LinkedHashMap<>();
        this.headers.put("subscription", subscriptionId);
        this.headers.put("message-id", String.valueOf(messageId));
        this.headers.put("destination", channel);
        this.messageBody = frame.getMessageBody();
    }

    /**
     * Adds a header to the frame.
     *
     * @param key   the header key
     * @param value the header value
     */
    public void addHeader(String key, String value) {
        headers.put(key, value);
    }

    /**
     * Retrieves the value of a specific header.
     *
     * @param key the header key
     * @return the value of the specified header, or {@code null} if the header does
     *         not exist
     */
    public String getHeader(String key) {
        return headers.get(key);
    }

    /**
     * Sets the message body of the frame.
     *
     * @param messageBody the new message body
     */
    public void setMessageBody(String messageBody) {
        this.messageBody = messageBody;
    }

    /**
     * Retrieves the message body of the frame.
     *
     * @return the message body
     */
    public String getMessageBody() {
        return messageBody;
    }

    /**
     * Retrieves the type of the frame.
     *
     * @return the type of the frame
     */
    public String getType() {
        return type;
    }

    /**
     * Serializes the frame into a STOMP-formatted string.
     *
     * @return the serialized STOMP frame as a string
     */
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

    /**
     * Converts the frame to a user-friendly string representation without the null
     * terminator.
     *
     * @return the frame as a readable string
     */
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

    /**
     * Parses a byte array into a {@code Frame} object.
     *
     * @param rawBytes the raw bytes representing a STOMP frame
     * @return the parsed {@code Frame} object
     * @throws IllegalArgumentException if the byte array is invalid or missing a
     *                                  frame type
     */
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

        // Parse message body (everything after the blank line until the null
        // terminator)
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
     * Serializes the frame into a byte array using UTF-8 encoding.
     *
     * @return the serialized frame as a byte array
     */
    public byte[] toBytes() {
        return this.toString().getBytes(StandardCharsets.UTF_8);
    }
}
