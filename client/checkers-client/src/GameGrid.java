import java.awt.GridLayout;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;

import java.awt.Color;
import java.util.ArrayList;


public class GameGrid extends JFrame {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	JPanel panel=new JPanel();
	GridButton buttons[];
	protected boolean locked = false;
	protected int timeLeft = 0;
	protected JLabel timeLabel;
	protected JLabel infoLabel;
	public JButton quitButton;
	public GridButton selectedButton = null;
	public ArrayList<GridButton> highlightedButtons = new ArrayList<GridButton>();
	
	
	public GameGrid(final Game listener){
		super("Checkers Game");
		int n = 8;
		buttons = new GridButton[32];
		panel.setLayout(new GridLayout(n+1,n));
		timeLabel = new JLabel("Time", SwingConstants.CENTER);
		//timeLabel.setForeground(Color.RED);
		//timeLabel.setText("Time left: " + 10);
		//panel.add(timeLabel);
		quitButton = new JButton();
		quitButton.setText("<html>Quit</html>");
		quitButton.addActionListener(listener);
		panel.add(quitButton);
		for (int i = 0; i < n-2; i++) {
			JLabel empty = new JLabel("", SwingConstants.CENTER);
			panel.add(empty);
		}
		infoLabel = new JLabel("Info", SwingConstants.CENTER);
		panel.add(infoLabel);
		int btnNum = 0;
		byte btnState = GridButton.Pawn1State;
		for(int i=0;i<n*n;i++){
			if ((i % 2 == 1 && (i / 8 % 2) == 0 ) || (i % 2 == 0 && (i / 8 % 2) == 1 )) { 
				if (i >= 24) {
					btnState = GridButton.EmptyState;
				}
				if (i >= 40) {
					btnState = GridButton.Pawn2State;
				}
				buttons[btnNum] = new GridButton(i % 2, i / 2, btnNum, btnState);
				buttons[btnNum].addActionListener(listener);
				panel.add(buttons[btnNum]);
				btnNum++;
			} else {
				//buttons[btnNum] = null;
				JLabel empty = new JLabel("", SwingConstants.CENTER);
				panel.add(empty);
			}
			
		}
		setSize(n * 100, (n+1)*100);
		setResizable(false);
		setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		addWindowListener(new java.awt.event.WindowAdapter() {
		    @Override
		    public void windowClosing(java.awt.event.WindowEvent windowEvent) {
		    	lock();
		        listener.killGame();
		    }
		});
		setVisible(true);
		add(panel);
	}
	
	public void lock() {
		locked = true;
		for (GridButton btn : highlightedButtons) {
			btn.setHighlighted(false);
		}
		highlightedButtons.clear();
		if (selectedButton != null) {
			selectedButton.setSelected(false);
			selectedButton = null;
		}
	}
	
	public void unlock() {
		locked = false;
	}
	
	public boolean isLocked() {
		return locked;
	}
	
	public void setInfo(String info) {
		infoLabel.setText(info);
	}
	
	public void setFieldState(int fieldNum, byte state) {
		buttons[fieldNum].setState(state);
	}
	
	public int getXCoords(int fieldNum) {
		if ((fieldNum / 4) % 2 == 0) {
	        return (fieldNum % 4) * 2 + 1;
	    }
	    return (fieldNum % 4) * 2;
	}
	
	public int getYCoords(int fieldNum) {
		return fieldNum / 4;
	}
	
	public int XYtoFieldNum(int x, int y) {
		return y * 4 + (x / 2);
	}
	
	public byte getStateByCoords(int x, int y) {
		return buttons[XYtoFieldNum(x, y)].getState();
	}
	
	public boolean areEnemies(int x1, int y1, int x2, int y2) {
		if ((getStateByCoords(x1, y1) == 1 || getStateByCoords(x1, y1) == 2) 
				&& (getStateByCoords(x2, y2) == 3 || getStateByCoords(x2, y2) == 4)) {
			return true;
		}
		if ((getStateByCoords(x1, y1) == 3 || getStateByCoords(x1, y1) == 4) 
				&& (getStateByCoords(x2, y2) == 1 || getStateByCoords(x2, y2) == 2)) {
			return true;
		}
		return false;
	}
	
	public boolean canGoDown(int fieldNum) {
		return buttons[fieldNum].canGoDown();
	}
	
	public boolean canGoUp(int fieldNum) {
		return buttons[fieldNum].canGoUp();
	}
	
	public boolean isMovePossible(int x1, int y1, int x2, int y2) {
		if (x2 < 0 || x2 >= 8 || y2 < 0 || y2 >= 8)
			return false;
		int fieldNum = XYtoFieldNum(x1, y1);
		if (y1 < y2) {
			if (!canGoDown(fieldNum)) {
				return false;
			}
		} else {
			if (!canGoUp(fieldNum)) {
				return false;
			}
		}
		if (Math.abs(x1 - x2) == 1 && Math.abs(y1 - y2) == 1) {
			if (getStateByCoords(x2, y2) == GridButton.EmptyState) {
				return true;
			}
		}
		if (Math.abs(x1 - x2) == 2 && Math.abs(y1 - y2) == 2) {
			int midx = (x1+x2) / 2;
			int midy = (y1+y2) / 2;
			if (getStateByCoords(x2, y2) == GridButton.EmptyState) {
				if (areEnemies(x1, y1, midx, midy)) {
					return true;
				}
			}
		}
		
		return false;
	}
	
	public void highlihtPossibleMoves(GridButton btn) {
		int fieldNum = btn.getFieldNum();
		if (btn.getState() == GridButton.EmptyState)
			return;
		int x = getXCoords(fieldNum);
		int y = getYCoords(fieldNum);
		int xn, yn;
		
		xn = x + 1;
		yn = y + 1;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
		}
		
		xn = x - 1;
		yn = y + 1;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
		}
		
		xn = x - 1;
		yn = y - 1;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
		}
		
		
		xn = x + 1;
		yn = y - 1;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
			System.out.println("Highlighting button: " + String.valueOf(XYtoFieldNum(xn, yn)));
		}
		
		
		

		xn = x + 2;
		yn = y + 2;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
		}
		
		xn = x - 2;
		yn = y + 2;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
		}
		
		xn = x - 2;
		yn = y - 2;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
		}
		
		
		xn = x + 2;
		yn = y - 2;
		if (isMovePossible(x, y, xn, yn)) {
			highlightedButtons.add(buttons[XYtoFieldNum(xn, yn)]);
			buttons[XYtoFieldNum(xn, yn)].setHighlighted(true);
		}
		
	}

}
