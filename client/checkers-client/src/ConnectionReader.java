import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;


public class ConnectionReader implements Runnable {
	
	private Game game;
	private String lastState = "";
	private BufferedReader reader;
	
	public ConnectionReader(Game game, InputStream is) {
		reader = new BufferedReader(new InputStreamReader(is));
		this.game = game;
		System.out.println("Reader started.");
	}

	public void run() {
		while (true) {
			try {
				String msg = reader.readLine();
				System.out.println("msg: " + msg);
				
				if (msg == null) {
					game.gameErrorEnd();
				}
				
				if (msg.startsWith("await")) {
					readAwait();
				} else if (msg.startsWith("start")) {
					readStart();
				} else if (msg.startsWith("state")) {
					readState(msg);
				} else if (msg.startsWith("end")) {
					readEnd(msg);
				}
			} catch (IOException e) {
				//System.out.println(e.getMessage());
				if (game.toDiscard == false) 
					game.gameErrorEnd();
				break;
			} catch (NullPointerException e) {
				break;
			}
			
		}
	}
	
	private void readState(String msg) {
		if (lastState != msg) {
			lastState = msg;
			game.updateState(msg);
		}
	}
	
	private void readStart() {
		game.startGame();
	}
	
	private void readAwait() {
		game.setPlayer((byte) 1);
	}
	
	private void readEnd(String msg) {
		//
	}

}
