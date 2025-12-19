using System;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;

public class AssemblerGUI : Form
{
    private RichTextBox inputTextBox; // Changed to RichTextBox
    private TextBox outputTextBox;
    private Button assembleButton;
    private Button loadFileButton;
    private Label statusLabel;
    private SplitContainer splitContainer;
    private bool isHighlighting = false;

    public AssemblerGUI()
    {
        // Form Setup
        this.Text = "TitanASM Studio"; // Renamed
        this.Size = new Size(900, 700);
        this.StartPosition = FormStartPosition.CenterScreen;

        // Split Container (Left: Code, Right: Object Code)
        splitContainer = new SplitContainer();
        splitContainer.Dock = DockStyle.Fill;
        splitContainer.Orientation = Orientation.Vertical;
        splitContainer.SplitterDistance = 450;

        // Input Area
        Label inputLabel = new Label() { Text = "Titan Source:", Dock = DockStyle.Top, Height = 20, Font = new Font("Segoe UI", 9, FontStyle.Bold) };
        inputTextBox = new RichTextBox(); // RichTextBox
        inputTextBox.Dock = DockStyle.Fill;
        inputTextBox.Font = new Font("Consolas", 11);
        inputTextBox.AcceptsTab = true;
        inputTextBox.WordWrap = false;
        inputTextBox.TextChanged += new EventHandler(Input_TextChanged);
        
        // Sample Code Buffer
        inputTextBox.Text = "; TitanASM Sample Code\r\norg 100h\r\nmain proc\r\n    printn \"Hello Titan\"\r\n    mov ah, 4Ch\r\n    int 21h\r\nmain endp";

        // Output Area
        Label outputLabel = new Label() { Text = "Machine Code Output:", Dock = DockStyle.Top, Height = 20, Font = new Font("Segoe UI", 9, FontStyle.Bold) };
        outputTextBox = new TextBox();
        outputTextBox.Multiline = true;
        outputTextBox.ScrollBars = ScrollBars.Vertical;
        outputTextBox.Dock = DockStyle.Fill;
        outputTextBox.ReadOnly = true;
        outputTextBox.Font = new Font("Consolas", 11);
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
        assembleButton.BackColor = Color.LightSkyBlue;
        assembleButton.Click += new EventHandler(AssembleButton_Click);

        loadFileButton = new Button();
        loadFileButton.Text = "Load File";
        loadFileButton.Location = new Point(120, 10);
        loadFileButton.Width = 100;
        loadFileButton.Click += new EventHandler(LoadFileButton_Click);
        
        Button runButton = new Button();
        runButton.Text = "Run Titan";
        runButton.Location = new Point(230, 10);
        runButton.Width = 100;
        runButton.BackColor = Color.LightGreen;
        runButton.Click += new EventHandler(RunButton_Click);

        statusLabel = new Label();
        statusLabel.Text = "Ready for Launch";
        statusLabel.Location = new Point(340, 15);
        statusLabel.AutoSize = true;
        statusLabel.ForeColor = Color.DarkBlue;

        // Add Controls
        bottomPanel.Controls.Add(assembleButton);
        bottomPanel.Controls.Add(loadFileButton);
        bottomPanel.Controls.Add(runButton);
        bottomPanel.Controls.Add(statusLabel);

        splitContainer.Panel1.Controls.Add(inputTextBox);
        splitContainer.Panel1.Controls.Add(inputLabel);
        splitContainer.Panel2.Controls.Add(outputTextBox);
        splitContainer.Panel2.Controls.Add(outputLabel);

        this.Controls.Add(splitContainer);
        this.Controls.Add(bottomPanel);
        
        // Initial Highlight
        HighlightSyntax();
    }

    private void Input_TextChanged(object sender, EventArgs e) {
        if (isHighlighting) return;
        HighlightSyntax();
    }

    private void HighlightSyntax() {
        isHighlighting = true;
        int selStart = inputTextBox.SelectionStart;
        int selLength = inputTextBox.SelectionLength;

        // Reset all text style
        inputTextBox.SelectAll();
        inputTextBox.SelectionColor = Color.Black;
        inputTextBox.SelectionFont = new Font(inputTextBox.Font, FontStyle.Regular);

        // Keywords
        string keywords = @"\b(LOAD|STORE|ADD|SUB|MULT|DIV|JMP|JZ|JNZ|HALT|ORG|DW|DB|MACRO|MEND|mov|printn|int|proc|endp)\b";
        MatchCollection keywordMatches = Regex.Matches(inputTextBox.Text, keywords, RegexOptions.IgnoreCase); // Added IgnoreCase
        foreach (Match m in keywordMatches) {
            inputTextBox.Select(m.Index, m.Length);
            inputTextBox.SelectionColor = Color.Blue;
            inputTextBox.SelectionFont = new Font(inputTextBox.Font, FontStyle.Bold);
        }

        // Numbers
        string numbers = @"\b\d+[hH]?\b"; // Updated for Hex suffix
        MatchCollection numberMatches = Regex.Matches(inputTextBox.Text, numbers);
        foreach (Match m in numberMatches) {
            inputTextBox.Select(m.Index, m.Length);
            inputTextBox.SelectionColor = Color.Magenta;
        }

        // Labels
        string labels = @"\w+:";
        MatchCollection labelMatches = Regex.Matches(inputTextBox.Text, labels);
        foreach (Match m in labelMatches) {
            inputTextBox.Select(m.Index, m.Length);
            inputTextBox.SelectionColor = Color.DarkRed;
        }

        // Comments (Must be last to override)
        string comments = @";.*$";
        MatchCollection commentMatches = Regex.Matches(inputTextBox.Text, comments, RegexOptions.Multiline);
        foreach (Match m in commentMatches) {
            inputTextBox.Select(m.Index, m.Length);
            inputTextBox.SelectionColor = Color.Green;
            inputTextBox.SelectionFont = new Font(inputTextBox.Font, FontStyle.Italic);
        }

        // Restore Selection
        inputTextBox.Select(selStart, selLength);
        inputTextBox.SelectionColor = Color.Black;
        inputTextBox.SelectionFont = new Font(inputTextBox.Font, FontStyle.Regular);

        isHighlighting = false;
    }

    private void RunButton_Click(object sender, EventArgs e)
    {
        string exePath = "TitanASM.exe"; // Renamed
        string objInternal = "result.obj";

        if (!File.Exists(objInternal)) {
            MessageBox.Show("Please assemble the code first!");
            return;
        }

        ProcessStartInfo startInfo = new ProcessStartInfo();
        startInfo.FileName = exePath;
        startInfo.Arguments = "-run \"" + objInternal + "\"";
        startInfo.RedirectStandardOutput = true;
        startInfo.RedirectStandardError = true;
        startInfo.UseShellExecute = false;
        startInfo.CreateNoWindow = true;

        try {
            using (Process process = Process.Start(startInfo)) {
                process.WaitForExit();
                string output = process.StandardOutput.ReadToEnd();
                string error = process.StandardError.ReadToEnd();
                
                MessageBox.Show(output + "\n" + error, "Titan Simulator");
            }
        } catch (Exception ex) {
            MessageBox.Show("Titan Error: " + ex.Message);
        }
    }

    private void AssembleButton_Click(object sender, EventArgs e)
    {
        string inputPath = "temp.asm";
        string outputPath = "result.obj";
        string exePath = "TitanASM.exe"; // Renamed

        // 1. Save input to file
        try {
            File.WriteAllText(inputPath, inputTextBox.Text); // RichTextBox .Text property
        } catch (Exception ex) {
            MessageBox.Show("Error saving temp file: " + ex.Message);
            return;
        }

        // 2. Check if assembler exists
        if (!File.Exists(exePath)) {
            MessageBox.Show("Error: TitanASM.exe not found in current directory!");
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
