import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.SwingConstants;
import javax.swing.JTextField;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.JButton;
import javax.swing.WindowConstants;

import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;

import java.awt.EventQueue;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.net.Socket;
import java.net.UnknownHostException;

public class MainMenu extends JFrame {
	
	private JLabel lblCheckersCleint;
	private static final long serialVersionUID = 1L;
	private JTextField textField;
	private JSpinner spinner;
	private JButton btnConnect;
	private JLabel lblinfo;
	private JLabel lblNewLabel;
	private JLabel lblNewLabel_1;
	
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					MainMenu frame = new MainMenu();
					frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}
	
	
	public MainMenu() {
		getContentPane().setLayout(null);
		this.setSize(480, 288);
		
		lblCheckersCleint = new JLabel("Checkers Client");
		lblCheckersCleint.setBounds(183, 5, 110, 15);
		lblCheckersCleint.setHorizontalAlignment(SwingConstants.CENTER);
		getContentPane().add(lblCheckersCleint);
		
		textField = new JTextField();
		textField.setBounds(63, 82, 170, 26);
		getContentPane().add(textField);
		textField.setColumns(10);
		
		spinner = new JSpinner();
		spinner.setModel(new SpinnerNumberModel(new Integer(1234), null, null, new Integer(1)));
		JSpinner.NumberEditor editor = new JSpinner.NumberEditor(spinner, "#");
		spinner.setEditor(editor);
		spinner.setBounds(301, 83, 77, 25);
		getContentPane().add(spinner);
		
		btnConnect = new JButton("Connect");
		btnConnect.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				updateInfo("Connecting...");
				btnConnect.setEnabled(false);
				//System.out.println(textField.getText() + " " + String.valueOf(spinner.getValue()));
				
				try {
					
					Socket socket = new Socket(textField.getText(), (int) spinner.getValue());
					
					InputStream is = socket.getInputStream();
					OutputStream os = socket.getOutputStream();
					
					new Game(is, os, socket);
					
					updateInfo("Connected.");
					btnConnect.setEnabled(true);
						
						
				}
					
					
				 catch (UnknownHostException ex) {
					updateInfo("Server not found.");
					btnConnect.setEnabled(true);
				} catch (IOException e1) {
					updateInfo("I/O error occured: " + e1.getMessage());
					btnConnect.setEnabled(true);
				}
				
			}
		});
		btnConnect.setBounds(176, 166, 117, 25);
		getContentPane().add(btnConnect);
		
		lblinfo = new JLabel("");
		lblinfo.setHorizontalAlignment(SwingConstants.CENTER);
		lblinfo.setBounds(22, 213, 442, 15);
		getContentPane().add(lblinfo);
		
		lblNewLabel = new JLabel("Server ip/hostname");
		lblNewLabel.setBounds(63, 61, 153, 15);
		getContentPane().add(lblNewLabel);
		
		lblNewLabel_1 = new JLabel("Port");
		lblNewLabel_1.setBounds(301, 61, 70, 15);
		getContentPane().add(lblNewLabel_1);
	}
	
	private void updateInfo(String info) {
		lblinfo.setText(info);
	}
}
