using UnityEngine;
using System.IO.Ports;

public class IMUReader : MonoBehaviour
{
    public static IMUReader Instance;
    SerialPort port;

    float targetRoll  = 0f;
    float targetPitch = 0f;

    // adjust these to fix hand direction
    // change signs (+/-) if hand moves opposite to real movement
    public float rollMultiplier  =  1f;
    public float pitchMultiplier = -1f;

    // smoothing — increase for faster response, decrease for smoother
    public float smoothSpeed = 10f;

    // rotation offset to fix hand facing wrong way
    // adjust Y value if hand faces wrong direction (try 180)
    public Vector3 rotationOffset = new Vector3(0, 180, 0);

    string portName = "COM5";
    int baudRate = 9600;

    bool isRunning = false;

    void Awake() { Instance = this; }

    void Start()
    {
        port = new SerialPort(portName, baudRate);
        port.ReadTimeout  = 20;
        port.WriteTimeout = 20;
        port.DtrEnable    = true;

        try
        {
            port.Open();
            isRunning = true;
            Debug.Log("Connected to Arduino on " + portName);
        }
        catch (System.Exception e)
        {
            Debug.LogError("Could not open port: " + e.Message);
        }
    }

    void Update()
    {
        if (port == null || !port.IsOpen) return;

        // drain all available lines this frame so nothing queues up
        // this prevents the freezing/lag issue
        try
        {
            while (port.BytesToRead > 0)
            {
                string line = port.ReadLine().Trim();

                if (line.StartsWith("R:"))
                {
                    string[] parts = line.Split(',');
                    if (parts.Length >= 2)
                    {
                        float r, p;
                        bool okR = float.TryParse(
                            parts[0].Replace("R:", ""),
                            System.Globalization.NumberStyles.Float,
                            System.Globalization.CultureInfo.InvariantCulture,
                            out r);
                        bool okP = float.TryParse(
                            parts[1].Replace("P:", ""),
                            System.Globalization.NumberStyles.Float,
                            System.Globalization.CultureInfo.InvariantCulture,
                            out p);

                        if (okR && okP)
                        {
                            targetRoll  = r * rollMultiplier;
                            targetPitch = p * pitchMultiplier;
                        }
                    }
                }
            }
        }
        catch (System.TimeoutException) { }
        catch (System.Exception e)
        {
            Debug.LogWarning("Read error: " + e.Message);
        }

        // apply rotation offset + smooth interpolation every frame
        Quaternion imuRotation = Quaternion.Euler(-targetPitch, 0, targetRoll);
        Quaternion offsetRotation = Quaternion.Euler(rotationOffset);
        Quaternion targetRotation = offsetRotation * imuRotation;

        transform.rotation = Quaternion.Lerp(
            transform.rotation,
            targetRotation,
            smoothSpeed * Time.deltaTime);
    }

    public void SendMotorCommand(string command)
    {
        if (port != null && port.IsOpen)
        {
            try
            {
                port.WriteLine(command);
                Debug.Log("Sent: " + command);
            }
            catch (System.Exception e)
            {
                Debug.LogWarning("Send error: " + e.Message);
            }
        }
        else
        {
            Debug.LogWarning("Port not open — cannot send " + command);
        }
    }

    void OnApplicationQuit()
    {
        isRunning = false;
        if (port != null && port.IsOpen)
        {
            port.WriteLine("OFF");
            port.Close();
        }
    }
}