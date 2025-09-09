using Ivi.Visa;
using Ivi.Visa.FormattedIO;
using System.Collections.ObjectModel;
using System.Diagnostics.Eventing.Reader;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using static System.Net.Mime.MediaTypeNames;

namespace ResponceTracer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public ObservableCollection<ViewData> myTable;
        IMessageBasedSession ? session;

        public MainWindow()
        {
            InitializeComponent();
            myTable = new ObservableCollection<ViewData>();
            data_grid.ItemsSource = myTable;
        }

        public class ViewData
        {
            public int ATT { get; set; }
            public string RESP { get; set; }

            public ViewData(int att, string resp ) 
            {
                ATT = att;
                RESP = resp;
            }
        }
        
        bool stop = false;

        private void btn_run_Click(object sender, RoutedEventArgs e)
        {
            if ((string)btn_run.Content == "Run")
            {
                btn_run.Content = "Stop";
                myTable.Clear();
                Measure(tb_serial.Text, tb_visa.Text);           
            }
            else
            {
                stop = true;
                btn_run.Content = "Stopping";
            }

        }

        void OnDataAcquisition(ViewData record)
        {
            myTable.Add(record);
            var lastItem =   myTable.Last(); // 最後の要素を取得します
            data_grid.SelectedItem = lastItem; // 最後の項目を選択状態にします
            data_grid.ScrollIntoView(lastItem); // 最後の項目までスクロールします
        }

        Task Measure(string serial, string  visa)
        {
            Progress<ViewData> progress = new Progress<ViewData>(OnDataAcquisition);
            Progress<string> done = new Progress<string>(OnMeasurementCompleted);


            SerialPort myPort; 

            return Task.Run(() => {
                try
                {
                    myPort = new SerialPort(serial, 9600, Parity.None, 8, StopBits.One);
                    myPort.Open();
                }
                catch (Exception ex) {
                    MessageBox.Show($"Couldn't open serial port,. Error is:\r\n{ex}");
                    return;
                }

                try
                {
                    session = GlobalResourceManager.Open(visa) as IMessageBasedSession;
                }
                catch (Exception visaException)
                {
                    MessageBox.Show($"Couldn't connect. Error is:\r\n{visaException}");
                    return;
                }
                MessageBasedFormattedIO formattedIO = new MessageBasedFormattedIO(session);

                if (session.ResourceName.Contains("ASRL") || session.ResourceName.Contains("SOCKET"))
                    session.TerminationCharacterEnabled = true;

                for (int i = 0; i < 256; i++)
                {
                    if (stop) continue;

                    myPort.WriteLine($"ATT:IMM {i}");
                    formattedIO.WriteLine("MEAS:VOLT:AC?");
                    string Response = formattedIO.ReadLine();
                    ((IProgress<ViewData>)progress).Report(new ViewData(i, Response.Replace("\r", "").Replace("\n", "")));
                }
                session.Dispose();
                myPort.Close();
                ((IProgress<string>)done).Report("done");

            }); 
            
        }

        private void OnMeasurementCompleted(string obj)
        {
            stop = false;
            btn_run.Content = "Run";
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            StringWriter sr = new StringWriter();
            foreach(ViewData data in myTable)
            {
                sr.WriteLine(data.ATT.ToString()+"\t" +data.RESP);
            }
            Clipboard.SetData(DataFormats.Text, sr);
        }
          
        private void menu_clear_Click(object sender, RoutedEventArgs e)
        {
            myTable.Clear();

        }
    }
}