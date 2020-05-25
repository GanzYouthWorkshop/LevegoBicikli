using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Android.App;
using Android.Content;
using Android.Hardware.Usb;
using Android.OS;
using Android.Runtime;
using Android.Support.Design.Widget;
using Android.Support.V7.App;
using Android.Views;
using Android.Webkit;
using Android.Widget;
using Hoho.Android.UsbSerial.Driver;
using Hoho.Android.UsbSerial.Extensions;
using Hoho.Android.UsbSerial.Util;
using Xamarin.Essentials;

namespace LevegoBicikli
{
    [Activity(Label = "@string/app_name", Theme = "@style/AppTheme.NoActionBar", MainLauncher = true)]
    public class MainActivity : AppCompatActivity
    {
        public const string EXTRA_TAG = "PortInfo";

        private UsbManager m_UsbManager;
        private UsbSerialPort m_SerialPort;
        private SerialInputOutputManager m_SerialIO;

        private WebView m_Web;

        protected override void OnCreate(Bundle savedInstanceState)
        {
            #region Acttivty init
            base.OnCreate(savedInstanceState);

            Platform.Init(this, savedInstanceState);

            this.RequestWindowFeature(WindowFeatures.NoTitle);
            this.SetContentView(Resource.Layout.activity_main);
            #endregion

            #region WebView init
            //HTML megjelenítő beállítása
            this.m_Web = this.FindViewById<WebView>(Resource.Id.webView);
            this.m_Web.Settings.JavaScriptCanOpenWindowsAutomatically = true;
            this.m_Web.Settings.JavaScriptEnabled = true;
            this.m_Web.LoadUrl("file:///android_asset/MainContent.html");
            #endregion
        }

        protected override void OnResume()
        {
            base.OnResume();

            #region USB adatkapcsoalt beállítása

            //Először elkérjük a komplett USB managert
            this.m_UsbManager = GetSystemService(Context.UsbService) as UsbManager;

            //Megkeressük a legelső (egyetlen) USB eszközt
            List<IUsbSerialDriver> devices = FindAllDriversAsync(this.m_UsbManager);

            IUsbSerialDriver hardware = devices.FirstOrDefault();
            if (hardware != null)
            {
                this.m_SerialPort = hardware.Ports.FirstOrDefault();
                if (this.m_SerialPort != null)
                {
                    //Ha találtunk eszközt, hozzáférést érünk az eszközhöz
                    PendingIntent pi = PendingIntent.GetBroadcast(this, 0, new Intent("ACTION_USB_PERMISSION"), 0);
                    this.m_UsbManager.RequestPermission(this.m_SerialPort.Driver.Device, pi);
                    //if(task.Result == true)
                    {
                        //Ha megkaptuk a hozzáférést akkor megynitjuk az eszközt mint sorosport
                        this.m_SerialIO = new SerialInputOutputManager(this.m_SerialPort)
                        {
                            BaudRate = 9600,
                            DataBits = 8,
                            StopBits = StopBits.One,
                            Parity = Parity.None,
                        };

                        this.m_SerialIO.DataReceived += (sender, e) =>
                        {
                            this.RunOnUiThread(() =>
                            {
                                this.OnReceiveSerialData(e.Data);
                            });
                        };

                        this.m_SerialIO.ErrorReceived += (sender, e) =>
                        {
                            this.RunOnUiThread(() =>
                            {
                                var intent = new Intent(this, typeof(MainActivity));
                                StartActivity(intent);
                            });
                        };

                        try
                        {
                            this.m_SerialIO.Open(this.m_UsbManager);

                            this.m_Web.EvaluateJavascript("ext_stateChanged('OK')", null);
                            return;
                        }
                        catch (Java.IO.IOException e)
                        {
                            this.m_Web.EvaluateJavascript("ext_stateChanged('ERROR')", null);
                            return;
                        }
                    }
                }

            }

            this.m_Web.EvaluateJavascript("ext_stateChanged('Unknown')", null);
            #endregion

        }
        public override bool OnCreateOptionsMenu(IMenu menu)
        {
            MenuInflater.Inflate(Resource.Menu.menu_main, menu);
            return true;
        }

        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
        {
            Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);

            base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
        }

        private static List<IUsbSerialDriver> FindAllDriversAsync(UsbManager usbManager)
        {
            ProbeTable table = UsbSerialProber.DefaultProbeTable;
            table.AddProduct(0x1b4f, 0x0008, typeof(CdcAcmSerialDriver)); // IOIO OTG
            table.AddProduct(0x09D8, 0x0420, typeof(CdcAcmSerialDriver)); // Elatec TWN4

            UsbSerialProber prober = new UsbSerialProber(table);

            return prober.FindAllDrivers(usbManager);
        }

        private void OnReceiveSerialData(byte[] data)
        {

            this.m_Web.EvaluateJavascript($"ext_dataReceived({this.SerialDataToInt(data)})", null);
        }

        private int SerialDataToInt(byte[] data)
        {
            string text = "";
            for(int i = 0; i < data.Length - 2; i++)
            {
                text += (char)data[i];
            }

            int value = 0;
            bool result = int.TryParse(text, out value);

            return value;
        }
    }
}
