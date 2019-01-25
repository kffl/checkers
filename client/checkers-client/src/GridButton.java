import javax.swing.ImageIcon;
import javax.swing.JButton;

import java.awt.Color;


public class GridButton extends JButton {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	ImageIcon icon;
	int num;
	byte state;
	public static byte EmptyState = 0;
	public static byte Pawn1State = 1;
	public static byte King1State = 2;
	public static byte Pawn2State = 3;
	public static byte King2State = 4;
	public boolean hightlighted;
	public boolean selected;
	public int fieldNum;
	private int xc,yc;
	protected ImageIcon[] icons = {null, 
	                               new ImageIcon(this.getClass().getResource("pawn1.png")),
	                               new ImageIcon(this.getClass().getResource("king1.png")),
	                               new ImageIcon(this.getClass().getResource("pawn2.png")),
	                               new ImageIcon(this.getClass().getResource("king2.png"))
								   };
	
	public GridButton(int x, int y, int fieldNum, byte state){
		this.xc = x;
		this.yc = y;
		this.fieldNum = fieldNum;
		setState(state);
		setHighlighted(false);
	}
	
	
	public void setState(byte state) {
		this.state = state;
		setIcon(icons[state]);
		
	}
	
	public void setHighlighted(boolean hightlighted) {
		this.hightlighted = hightlighted;
		if (hightlighted) {
			setBackground(Color.RED);
		} else {
			setBackground(Color.GRAY);
		}
	}
	
	public boolean isHighlighted() {
		return hightlighted;
	}
	
	public void setSelected(boolean selected) {
		this.selected = selected;
	}
	
	public int getXCoord() {
		return xc;
	}
	
	public int getYCoord() {
		return yc;
	}
	
	public int getFieldNum() {
		return fieldNum;
	}
	
	public byte getState() {
		return state;
	}
	
	public boolean canGoDown() {
		if (state == Pawn1State || state == King1State || state == King2State)
			return true;
		return false;
	}
	
	public int whoseMove() {
		if (state == Pawn1State || state == King1State)
			return 1;
		if (state == Pawn2State || state == King2State)
			return 2;
		return 0;
	}
	
	public boolean canGoUp() {
		if (state == King1State || state == Pawn2State || state == King2State)
			return true;
		return false;
	}
}
