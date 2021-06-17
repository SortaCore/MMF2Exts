package Extensions;
import Extensions.CRunBluewing_Client;

// JavaVM shutdown handler. Fusion doesn't appear to have a way of notifying native exts if app is closing down.
public class CRunBluewing_ClientShutdownThread extends Thread
{
	public void run()
	{
		CRunBluewing_Client.darkedif_EndApp();
	}
}
