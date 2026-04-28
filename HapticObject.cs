using UnityEngine;

public class HapticObject : MonoBehaviour
{
    void OnMouseEnter()
    {
        Debug.Log("Mouse entered sphere");
        if (IMUReader.Instance != null)
            IMUReader.Instance.SendMotorCommand("ON");
    }

    void OnMouseExit()
    {
        Debug.Log("Mouse exited sphere");
        if (IMUReader.Instance != null)
            IMUReader.Instance.SendMotorCommand("OFF");
    }

    void OnMouseDown()
    {
        Debug.Log("Mouse clicked sphere");
        if (IMUReader.Instance != null)
            IMUReader.Instance.SendMotorCommand("PULSE");
    }
}