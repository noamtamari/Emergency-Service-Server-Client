package bgu.spl.net.impl.stomp;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import bgu.spl.net.api.MessageEncoderDecoder;

public class StompEncoderDecoder implements MessageEncoderDecoder<String> {

    private byte[] bytes = new byte[1 << 10]; // Start with 1KB buffer
    private int len = 0;

    /**
     * Decodes the next byte of the message.
     *
     * @param nextByte the next byte to consider for the currently decoded message
     * @return a complete message if the null terminator is reached, or null otherwise
     */
    @Override
    public String decodeNextByte(byte nextByte) {
        if (nextByte == '\u0000') { // STOMP messages are terminated with the null character
            return popString(); // Return the accumulated string and reset the buffer
        }
        pushByte(nextByte); // Add the byte to the buffer
        return null; // Message is not completed yet
    }

    /**
     * Encodes the given message to a byte array.
     *
     * @param message the message to encode
     * @return the encoded byte array
     */
    @Override
    public byte[] encode(String message) {
        return (message + "\u0000").getBytes(StandardCharsets.UTF_8);
    }

    /**
     * Adds a byte to the buffer. Resizes the buffer if necessary.
     *
     * @param nextByte the byte to add to the buffer
     */
    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2); // Double the buffer size if full
        }
        bytes[len++] = nextByte;
    }

    /**
     * Extracts the accumulated string from the buffer and resets the buffer.
     *
     * @return the decoded string
     */
    private String popString() {
        String result = new String(bytes, 0, len, StandardCharsets.UTF_8); // Decode UTF-8 string from the buffer
        len = 0; // Reset the buffer for the next message
        return result;
    }
}
