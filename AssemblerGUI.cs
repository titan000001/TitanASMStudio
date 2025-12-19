using System;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;

public class AssemblerGUI : Form
{
    private TextBox inputTextBox;
    private TextBox outputTextBox;
    private Button assembleButton;
    private Button loadFileButton;
    private Label statusLabel;
    private SplitContainer splitContainer;

    public AssemblerGUI()
    {
        // Form Setup
        this.Text = "Simple Assembler IDE";
        this.Size = new Size(800, 600);
        this.StartPosition = FormStartPosition.CenterScreen;

        // Split Container (Left: Code, Right: Object Code)
        splitContainer = new SplitContainer();
        splitContainer.Dock = DockStyle.Fill;
        splitContainer.Orientation = Orientation.Vertical;
        splitContainer.SplitterDistance = 400;

        // Input Area
        Label inputLabel = new Label() { Text = "Assembly Source:", Dock = DockStyle.Top, Height = 20, Font = new Font("Segoe UI", 9, FontStyle.Bold) };
        inputTextBox = new TextBox();
        inputTextBox.Multiline = true;
        inputTextBox.ScrollBars = ScrollBars.Vertical;
        inputTextBox.Dock = DockStyle.Fill;
        inputTextBox.Font = new Font("Consolas", 10);
        
        // Sample Code Buffer
        inputTextBox.Text = "; Write your assembly code here\r\nORG 10\r\nSTART: LOAD 50\r\n       ADD 51\r\n       STORE 52\r\n       HALT\r\n\r\nORG 50\r\nDATA1: DW 5\r\nDATA2: DW 10\r\nRES:   DW 0";

        // Output Area
        Label outputLabel = new Label() { Text = "Object Code Output:", Dock = DockStyle.Top, Height = 20, Font = new Font("Segoe UI", 9, FontStyle.Bold) };
        outputTextBox = new TextBox();
        outputTextBox.Multiline = true;
        outputTextBox.ScrollBars = ScrollBars.Vertical;
        outputTextBox.Dock = DockStyle.Fill;
        outputTextBox.ReadOnly = true;
        outputTextBox.Font = new Font("Consolas", 10);
        outputTextBox.BackColor = Color.WhiteSmoke;

        // Controls Panel (Bottom)
        Panel bottomPanel = new Panel();
        bottomPanel.Height = 50;
        bottomPanel.Dock = DockStyle.Bottom;
        bottomPanel.BackColor = Color.LightGray;

        assembleButton = new Button();
        assembleButton.Text = "Assemble";
        assembleButton.Location = new Point(10, 10);
        assembleButton.Width = 100;
        assembleButton.Click += new EventHandler(AssembleButton_Click);

        loadFileButton = new Button();
        loadFileButton.Text = "Load File";
        loadFileButton.Location = new Point(120, 10);
        loadFileButton.Width = 100;
        loadFileButton.Click += new EventHandler(LoadFileButton_Click);

        statusLabel = new Label();
        statusLabel.Text = "Ready";
        statusLabel.Location = new Point(230, 15);
        statusLabel.AutoSize = true;
        statusLabel.ForeColor = Color.DarkBlue;

        // Add Controls
        bottomPanel.Controls.Add(assembleButton);
        bottomPanel.Controls.Add(loadFileButton);
        bottomPanel.Controls.Add(statusLabel);

        splitContainer.Panel1.Controls.Add(inputTextBox);
        splitContainer.Panel1.Controls.Add(inputLabel);
        splitContainer.Panel2.Controls.Add(outputTextBox);
        splitContainer.Panel2.Controls.Add(outputLabel);

        this.Controls.Add(splitContainer);
        this.Controls.Add(bottomPanel);
    }

    private void AssembleButton_Click(object sender, EventArgs e)
    {
        string inputPath = "temp.asm";
        string outputPath = "result.obj";
        string exePath = "assembler.exe";

        // 1. Save input to file
        try {
            File.WriteAllText(inputPath, inputTextBox.Text);
        } catch (Exception ex) {
            MessageBox.Show("Error saving temp file: " + ex.Message);
            return;
        }

        // 2. Check if assembler exists
        if (!File.Exists(exePath)) {
            MessageBox.Show("Error: assembler.exe not found in current directory!");
            return;
        }

        // 3. Run Assembler Process
        ProcessStartInfo startInfo = new ProcessStartInfo();
        startInfo.FileName = exePath;
        startInfo.Arguments = "\"" + inputPath + "\" \"" + outputPath + "\"";
        startInfo.RedirectStandardOutput = true;
        startInfo.RedirectStandardError = true;
        startInfo.UseShellExecute = false;
        startInfo.CreateNoWindow = true;

        try {
            using (Process process = Process.Start(startInfo)) {
                process.WaitForExit();
                string output = process.StandardOutput.ReadToEnd();
                string error = process.StandardError.ReadToEnd();

                if (process.ExitCode == 0) {
                    statusLabel.Text = "Success!";
                    statusLabel.ForeColor = Color.Green;
                    
                    // 4. Read Output
                    if (File.Exists(outputPath)) {
                        outputTextBox.Text = File.ReadAllText(outputPath);
                    }
                } else {
                    statusLabel.Text = "Failed!";
                    statusLabel.ForeColor = Color.Red;
                    outputTextBox.Text = "Error Log:\r\n" + error + "\r\n" + output;
                }
            }
        } catch (Exception ex) {
             MessageBox.Show("Error running assembler: " + ex.Message);
        }
    }

    private void LoadFileButton_Click(object sender, EventArgs e)
    {
        OpenFileDialog openFileDialog = new OpenFileDialog();
        openFileDialog.Filter = "Assembly Files|*.asm|All Files|*.*";
        
        if (openFileDialog.ShowDialog() == DialogResult.OK) {
            try {
                inputTextBox.Text = File.ReadAllText(openFileDialog.FileName);
                statusLabel.Text = "Loaded: " + Path.GetFileName(openFileDialog.FileName);
            } catch (Exception ex) {
                MessageBox.Show("Error reading file: " + ex.Message);
            }
        }
    }

    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new AssemblerGUI());
    }
}
