package bgu.spl.net.srv;

import java.util.concurrent.ConcurrentHashMap;

public class UserHandler {

    private static class UserHandlerHolder{
        private static final UserHandler instance = new UserHandler();
    }

    // The map of users (username -> password)
    private final ConcurrentHashMap<String, String> allUsers;
    private final ConcurrentHashMap<String, Integer> activeUsersByUserName;
    private final ConcurrentHashMap<Integer, String> activeUsersByConnectionId;

    // Private constructor to prevent direct instantiation
    private UserHandler() {
        allUsers = new ConcurrentHashMap<>();
        activeUsersByUserName = new ConcurrentHashMap<>();
        activeUsersByConnectionId = new ConcurrentHashMap<>();
    }

    // Public method to provide access to the singleton instance
    public static UserHandler getInstance() {
        return UserHandlerHolder.instance;
    }

    // Method to add a new user
    public void addNewUser(String username, String password, int connectionId) {
        allUsers.put(username, password);
        activeUsersByUserName.put(username, connectionId);
        activeUsersByConnectionId.put(connectionId, username);
    }

    // Method to add a new user
    public boolean logInUser(String username, String password, int connectionId) {
        if (validPassword(username,password) == true){
            activeUsersByUserName.put(username, connectionId);
            activeUsersByConnectionId.put(connectionId, username);
            return true;
        }
        return false;
    }

    // Method to check if a user exists
    public boolean userExists(String username) {
        return allUsers.containsKey(username);
    }

    // Method to check if a user exists
    public boolean IsUserLogedIn(String username) {
        return activeUsersByUserName.containsKey(username);
    }

    // Method to validate user credentials
    public boolean validPassword(String username, String password) {
        return password.equals(allUsers.get(username));
    }

    // Method to make user loged out
    public void removeActiveUser(int connectionId) {
        String username = activeUsersByConnectionId.remove(connectionId);
        if (username != null)
            activeUsersByUserName.remove(username);
    }
}
