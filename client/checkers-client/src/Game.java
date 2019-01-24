import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import javax.swing.JOptionPane;


public class Game implements ActionListener {

	public static Game currentGame = null;
	protected byte player;
	protected boolean gameAlive;
	private int moveCount;
	public GameGrid grid;
	protected byte status;
	private ConnectionWriter writer;
	private ConnectionReader reader;
	private Thread readerThread;
	public Socket socket;
	public boolean toDiscard = false;
	
	
	public Game(InputStream is, OutputStream os, Socket socket) {
		if (currentGame == null) {
			this.socket = socket;
			player = 2;
			moveCount = 1;
			grid = new GameGrid(this);
			grid.lock();
			writer = new ConnectionWriter(os);
			reader = new ConnectionReader(this, is);
			readerThread = new Thread(reader);
			readerThread.start();
			grid.setInfo("<html>Awaiting<br>game<br>start</html>");
		} else {
			JOptionPane.showMessageDialog(null, "Game is already running.", "Error", JOptionPane.PLAIN_MESSAGE);
		}
	}
	
	public void actionPerformed(ActionEvent e) {
		if (e.getSource() instanceof GridButton) {
			if (!grid.isLocked()) {
				GridButton button = (GridButton)e.getSource();
				if (!button.isHighlighted()) {
					grid.lock();
					grid.highlihtPossibleMoves(button);
					grid.selectedButton = button;
					System.out.println(button.getFieldNum());
					grid.unlock();
				} else {
					writer.sendMove(grid.selectedButton.getFieldNum(), button.getFieldNum());		
					grid.lock();
					//make move
				}
			}			
		} else {
			if (e.getSource() == grid.quitButton) {
				this.toDiscard = true;
				writer.sendQuit();
				System.out.println("Killing game");
				killGame();
			}
		}
	}
	
	public void startGame() {
		if (player == 1) {
			grid.unlock();
			grid.setInfo("<html>Game<br>started</html>");
		}
	}
	
	public void killGame() { 
		toDiscard = true;
		if (true) {
			grid.lock();
			String info = "";
			System.out.println(status);
			if (status == 6) {
					info = "Player 1 left.";
			} else if (status == 7) {
					info = "Player 2 left.";
			} else if (status == 5) {
				info = "It's a draw.";
			} else if ((status == 3 && player == 1) || (status == 4 && player == 2)) {
				info = "You won.";
			} else if ((status == 3 && player == 2) || (status == 4 && player == 1)) {
				info = "You lost.";
			} else {
				info = "Game over.";
			}
				JOptionPane.showOptionDialog(null, info, "Game over", JOptionPane.PLAIN_MESSAGE, JOptionPane.INFORMATION_MESSAGE, null, null, null);
			}
		currentGame = null;
		grid.setVisible(false);
		grid.dispose();
		try {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				//e.printStackTrace();
			}
			socket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
		}
	}
	
	public void gameErrorEnd() {
		grid.lock();
		JOptionPane.showOptionDialog(null, "Something went wrong. You were disconnected.", "Connection error", JOptionPane.PLAIN_MESSAGE, JOptionPane.INFORMATION_MESSAGE, null, null, null);
		currentGame = null;
		grid.setVisible(false);
		grid.dispose();
		try {
			socket.close();
		} catch (IOException e) {
			//
		}
	}
	
	public void setPlayer(byte player) {
		this.player = player;
	}
	
	public void updateState(String state) {
		grid.lock();
		state = state.substring(6);
		System.out.println(state);
		String[] params = state.split(";");
		status = Byte.valueOf(params[0]);
		System.out.println(status);
		moveCount = Byte.valueOf(params[1]);
		byte fieldState;
		for (int i = 2; i < params.length; i++) {
			fieldState = Byte.valueOf(params[i]);
			grid.setFieldState(i-2, fieldState);
		}
		if ((status == 1 && player == 1) || (status == 2 && player == 2)) {
			grid.setInfo("<html>Your<br>move</html>");
		}
		if ((status == 1 && player == 2) || (status == 2 && player == 1)) {
			grid.setInfo("<html>Opponent's<br>move</html>");
		}
		
		if (status != 1 && status != 2) {
			killGame();
		} else {
			grid.unlock();
		}
		
	}

}
