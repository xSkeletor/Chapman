import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

/**
*  SMTP Email Program.
*  Prompts a user for email information and data.
*  Creates a socket connecting to the Chapman SMTP Server.
*  Sends an email using the information provided.
*  Closes the socket and exits.
*  author: Dennis Fomichev
*  Email:  fomichev@chapman.edu
*  Date:  2/17/2025
*  version: 1.0
*/

class Email {

  public static void main(String[] argv) throws Exception {
    // Get user input
    BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in));
    System.out.print("Type the from address: ");
    final String fromAddress = inFromUser.readLine();

    System.out.print("Type the to address: ");
    final String toAddress = inFromUser.readLine();

    System.out.print("Type the sender name: ");
    final String senderName = inFromUser.readLine();

    System.out.print("Type the receiver name: ");
    final String receiverName = inFromUser.readLine();

    System.out.print("Type the subject: ");
    final String subject = inFromUser.readLine();

    System.out.print("Type the body (end with a .): ");
    String body = "";
    String line = inFromUser.readLine();

    while (!line.equals(".")) {
      body += line + "\n";

      line = inFromUser.readLine();
    }

    // Finished getting user input

    // Connect to the server
    Socket clientSocket = null;

    try {
      clientSocket = new Socket("smtp.chapman.edu", 25);
    } catch (Exception e) {
      System.out.println("Failed to open socket connection");
      System.exit(0);
    }
    PrintWriter outToServer = new PrintWriter(clientSocket.getOutputStream(), true);
    BufferedReader inFromServer =  new BufferedReader(
        new InputStreamReader(clientSocket.getInputStream()));

    // Exchange messages with the server
    // Recive and display the Welcome Message
    String welcomeMessage = inFromServer.readLine();
    System.out.println("SERVER:" + welcomeMessage);

    // Send the HELO and display the response
    System.out.println("CLIENT: HELO jenkins.chapman.edu");
    outToServer.println("HELO jenkins.chapman.edu");
    String modifiedSentence = inFromServer.readLine();
    System.out.println("SERVER: " + modifiedSentence);

    // Send the MAIL FROM and display the response
    System.out.println("CLIENT: MAIL FROM: " + fromAddress);
    outToServer.println("MAIL FROM: " + fromAddress);
    modifiedSentence = inFromServer.readLine();
    System.out.println("SERVER: " + modifiedSentence);

    // Send the RCPT TO and display the response
    System.out.println("CLIENT: RCPT TO: " + toAddress);
    outToServer.println("RCPT TO: " + toAddress);
    modifiedSentence = inFromServer.readLine();
    System.out.println("SERVER: " + modifiedSentence);

    // Send the DATA and display the response
    System.out.println("CLIENT: DATA");
    outToServer.println("DATA");
    modifiedSentence = inFromServer.readLine();
    System.out.println("SERVER: " + modifiedSentence);

    // Send the sender name and display the response
    System.out.println("CLIENT: From: " + senderName);
    outToServer.println("From: " + senderName);

    // Send the receiver name and display the response
    System.out.println("CLIENT: To: " + receiverName);
    outToServer.println("To: " + receiverName);

    // Send the subject and display the response
    System.out.println("CLIENT: Subject: " + subject);
    outToServer.println("Subject: " + subject);

    // Send the body and display the response
    System.out.println("CLIENT: " + body);
    outToServer.println(body);

    // Send the . and display the response
    System.out.println("CLIENT: .");
    outToServer.println(".");
    modifiedSentence = inFromServer.readLine();
    System.out.println("SERVER: " + modifiedSentence);

    // Send the QUIT message and display the response
    System.out.println("CLIENT: QUIT");
    outToServer.println("QUIT");
    modifiedSentence = inFromServer.readLine();
    System.out.println("SERVER: " + modifiedSentence);

    // Close the socket connection
    clientSocket.close();
  }
}