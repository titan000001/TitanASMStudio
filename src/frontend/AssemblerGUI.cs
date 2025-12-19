using System;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;

public class AssemblerGUI : Form
{
    private RichTextBox inputTextBox; 
    private TextBox outputTextBox;
    private Button assembleButton;
    private Button loadFileButton;
    private Label statusLabel;
    private SplitContainer splitContainer;
    private bool isHighlighting = false;

    // Debugger Controls
    private Process debugProcess;
    private Button debugButton, stepButton, stopButton;
    private Label axLabel, bxLabel, cxLabel, dxLabel, spLabel, ipLabel;

    public AssemblerGUI()
    {
        // Form Setup
        this.Text = "TitanASM Studio";
        this.Size = new Size(1100, 700); // Widen for debug panel
        this.StartPosition = FormStartPosition.CenterScreen;

        // Split Container (Left: Code, Right: Object Code)
        splitContainer = new SplitContainer();
        splitContainer.Dock = DockStyle.Fill;
        splitContainer.Orientation = Orientation.Vertical;
        splitContainer.SplitterDistance = 550;

        // Input Area
        Label inputLabel = new Label() { Text = "Titan Source:", Dock = DockStyle.Top, Height = 20, Font = new Font("Segoe UI", 9, FontStyle.Bold) };
        inputTextBox = new RichTextBox();
        inputTextBox.Dock = DockStyle.Fill;
        inputTextBox.Font = new Font("Consolas", 11);
        inputTextBox.AcceptsTab = true;
        inputTextBox.WordWrap = false;
        inputTextBox.TextChanged += new EventHandler(Input_TextChanged);
        
        // Sample Code Buffer
        inputTextBox.Text = "; TitanASM Debug Demo\r\norg 100h\r\n\r\nmain proc\r\n    mov ax, 0\r\n    call my_func\r\n    add ax, 1\r\n    mov ah, 4Ch\r\n    int 21h\r\nmain endp\r\n\r\nmy_func:\r\n    mov ax, 5\r\n    ret";

        // Output Area
        Label outputLabel = new Label() { Text = "Machine Code Output:", Dock = DockStyle.Top, Height = 20, Font = new Font("Segoe UI", 9, FontStyle.Bold) };
        outputTextBox = new TextBox();
        outputTextBox.Multiline = true;
        outputTextBox.ScrollBars = ScrollBars.Vertical;
        outputTextBox.Dock = DockStyle.Fill;
        outputTextBox.ReadOnly = true;
        outputTextBox.Font = new Font("Consolas", 11);
        outputTextBox.BackColor = Color.WhiteSmoke;

        // Debug Panel (Right Side of Form, Dock Right)
        Panel debugPanel = new Panel();
        debugPanel.Dock = DockStyle.Right;
        debugPanel.Width = 200;
        debugPanel.BackColor = Color.FromArgb(40, 40, 40);
        
        Label regHeader = new Label() { Text = "REGISTERS", ForeColor = Color.White, Top = 10, Left = 10, AutoSize = true, Font = new Font("Consolas", 12, FontStyle.Bold) };
        debugPanel.Controls.Add(regHeader);
        
        axLabel = CreateRegLabel("AX: 0000", 40);
        bxLabel = CreateRegLabel("BX: 0000", 70);
        cxLabel = CreateRegLabel("CX: 0000", 100);
        dxLabel = CreateRegLabel("DX: 0000", 130);
        spLabel = CreateRegLabel("SP: FFFE", 160);
        ipLabel = CreateRegLabel("IP: 0100", 190);
        
        debugPanel.Controls.Add(axLabel);
        debugPanel.Controls.Add(bxLabel);
        debugPanel.Controls.Add(cxLabel);
        debugPanel.Controls.Add(dxLabel);
        debugPanel.Controls.Add(spLabel);
        debugPanel.Controls.Add(ipLabel);

        stepButton = new Button() { Text = "Step Into", Top = 250, Left = 10, Width = 180, Height = 40, BackColor = Color.Yellow, Enabled = false };
        stepButton.Click += (s, ev) => SendDebugCommand("s");
        debugPanel.Controls.Add(stepButton);

        stopButton = new Button() { Text = "Stop", Top = 300, Left = 10, Width = 180, Height = 40, BackColor = Color.Red, Enabled = false };
        stopButton.Click += (s, ev) => StopDebug();
        debugPanel.Controls.Add(stopButton);

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
        runButton.Text = "Run";
        runButton.Location = new Point(230, 10);
        runButton.Width = 100;
        runButton.BackColor = Color.LightGreen;
        runButton.Click += new EventHandler(RunButton_Click);

        Button startDebugBtn = new Button();
        startDebugBtn.Text = "Debug";
        startDebugBtn.Location = new Point(340, 10);
        startDebugBtn.Width = 100;
        startDebugBtn.BackColor = Color.Orange;
        startDebugBtn.Click += new EventHandler(StartDebug_Click);

        statusLabel = new Label();
        statusLabel.Text = "Ready for Launch";
        statusLabel.Location = new Point(450, 15);
        statusLabel.AutoSize = true;
        statusLabel.ForeColor = Color.DarkBlue;

        // Add Controls
        bottomPanel.Controls.Add(assembleButton);
        bottomPanel.Controls.Add(loadFileButton);
        bottomPanel.Controls.Add(runButton);
        bottomPanel.Controls.Add(startDebugBtn);
        bottomPanel.Controls.Add(statusLabel);

        splitContainer.Panel1.Controls.Add(inputTextBox);
        splitContainer.Panel1.Controls.Add(inputLabel);
        splitContainer.Panel2.Controls.Add(outputTextBox);
        splitContainer.Panel2.Controls.Add(outputLabel);

        this.Controls.Add(splitContainer);
        this.Controls.Add(bottomPanel);
        this.Controls.Add(debugPanel); // Add Debug Panel
        
        // Initial Highlight
        HighlightSyntax();
    }

    private Label CreateRegLabel(string text, int top) {
        return new Label() { Text = text, Top = top, Left = 10, ForeColor = Color.Cyan, Font = new Font("Consolas", 11), AutoSize = true };
    }

    private void StartDebug_Click(object sender, EventArgs e) {
        string exePath = "TitanASM.exe";
        string objInternal = "result.obj";
        
        if (!File.Exists(objInternal)) { MessageBox.Show("Assemble first!"); return; }

        ProcessStartInfo startInfo = new ProcessStartInfo();
        startInfo.FileName = exePath;
        startInfo.Arguments = "-debug \"" + objInternal + "\"";
        startInfo.RedirectStandardOutput = true;
        startInfo.RedirectStandardInput = true;
        startInfo.RedirectStandardError = true;
        startInfo.UseShellExecute = false;
        startInfo.CreateNoWindow = true;

        debugProcess = new Process();
        debugProcess.StartInfo = startInfo;
        debugProcess.OutputDataReceived += DebugOutputHandler;
        debugProcess.BeginOutputReadLine(); // Async read
        debugProcess.Start();

        stepButton.Enabled = true;
        stopButton.Enabled = true;
        statusLabel.Text = "Debugging Started...";
    }

    private void StopDebug() {
        if (debugProcess != null && !debugProcess.HasExited) {
            try {
                debugProcess.StandardInput.WriteLine("q");
                debugProcess.WaitForExit(1000);
            } catch {}
            if (!debugProcess.HasExited) debugProcess.Kill();
        }
        stepButton.Enabled = false;
        stopButton.Enabled = false;
        statusLabel.Text = "Debugging Stopped.";
    }

    private void SendDebugCommand(string cmd) {
        if (debugProcess != null && !debugProcess.HasExited) {
            debugProcess.StandardInput.WriteLine(cmd);
        }
    }

    private void DebugOutputHandler(object sendingProcess, DataReceivedEventArgs outLine) {
        if (String.IsNullOrEmpty(outLine.Data)) return;
        string line = outLine.Data.Trim();
        // Parse DEBUG|IP|AX...
        if (line.StartsWith("DEBUG|")) {
            string[] parts = line.Split('|');
            // Update UI (Invoke required)
            this.Invoke((MethodInvoker)delegate {
                if (parts.Length > 6) {
                    ipLabel.Text = "IP: " + parts[1];
                    axLabel.Text = "AX: " + parts[2];
                    bxLabel.Text = "BX: " + parts[3];
                    cxLabel.Text = "CX: " + parts[4];
                    dxLabel.Text = "DX: " + parts[5];
                    spLabel.Text = "SP: " + parts[6];
                }
            });
        }
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
        string keywords = @"\b(LOAD|STORE|ADD|SUB|MULT|DIV|JMP|JZ|JNZ|HALT|ORG|DW|DB|MACRO|MEND|mov|printn|int|push|pop|call|ret|proc|endp)\b";
        MatchCollection keywordMatches = Regex.Matches(inputTextBox.Text, keywords, RegexOptions.IgnoreCase);
        foreach (Match m in keywordMatches) {
            inputTextBox.Select(m.Index, m.Length);
            inputTextBox.SelectionColor = Color.Blue;
            inputTextBox.SelectionFont = new Font(inputTextBox.Font, FontStyle.Bold);
        }

        // Numbers
        string numbers = @"\b\d+[hH]?\b";
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

        // Comments
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
        string exePath = "TitanASM.exe"; 
        string objInternal = "result.obj";

        if (!File.Exists(objInternal)) {
            MessageBox.Show("Please assemble the code first!");
            return;
        }

        // Launch in new Console Window to allow Input
        ProcessStartInfo startInfo = new ProcessStartInfo();
        startInfo.FileName = "cmd.exe";
        startInfo.Arguments = "/K \"" + exePath + " -run \"" + objInternal + "\"\"";
        startInfo.UseShellExecute = true; // Use Shell to spawn window
        startInfo.CreateNoWindow = false; // Show Window

        try {
            Process.Start(startInfo);
        } catch (Exception ex) {
            MessageBox.Show("Titan Error: " + ex.Message);
        }
    }

    private void AssembleButton_Click(object sender, EventArgs e)
    {
        string inputPath = "temp.asm";
        string outputPath = "result.obj";
        string exePath = "TitanASM.exe"; 

        try {
            File.WriteAllText(inputPath, inputTextBox.Text); 
        } catch (Exception ex) {
            MessageBox.Show("Error saving temp file: " + ex.Message);
            return;
        }

        if (!File.Exists(exePath)) {
            MessageBox.Show("Error: TitanASM.exe not found!");
            return;
        }

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
                    statusLabel.Text = "Assembly Success!";
                    statusLabel.ForeColor = Color.Green;
                    
                    if (File.Exists(outputPath)) {
                        outputTextBox.Text = File.ReadAllText(outputPath);
                    }
                } else {
                    statusLabel.Text = "Assembly Failed!";
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
