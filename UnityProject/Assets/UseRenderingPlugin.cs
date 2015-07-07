// on OpenGL ES there is no way to query texture extents from native texture id
#if (UNITY_IPHONE || UNITY_ANDROID) && !UNITY_EDITOR
	#define UNITY_GLES_RENDERER
#endif


using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;


public class UseRenderingPlugin : MonoBehaviour
{
	// Native plugin rendering events are only called if a plugin is used
	// by some script. This means we have to DllImport at least
	// one function in some active script.
	// For this example, we'll call into plugin's SetTimeFromUnity
	// function and pass the current time so the plugin can animate.

#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
	private static extern void SetTimeFromUnity(float t);


	// We'll also pass native pointer to a texture in Unity.
	// The plugin will fill texture data from native code.
#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
	private static extern void SetTextureFromUnity(System.IntPtr texture, int w, int h);


	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin")]
	#endif
	private static extern void SetMatricesFromUnity( float[] modelMatrix, float[] viewMatrix, float[] projectionMatrix );


	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin")]
	#endif
	private static extern void InitPlugin ();


	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin")]
	#endif
	private static extern char[] getOpenGLErrorsLog ();



	IEnumerator Start () {
		string errorLog = new string( getOpenGLErrorsLog() );
		Debug.Log( errorLog );
		InitPlugin ();
		Debug.Log( new string( getOpenGLErrorsLog() ) );
		CreateTextureAndPassToPlugin();
		yield return StartCoroutine("CallPluginAtEndOfFrames");
	}
	private void CreateTextureAndPassToPlugin()
	{
		// Create a texture
		Texture2D tex = new Texture2D(256,256,TextureFormat.ARGB32,false);
		// Set point filtering just so we can see the pixels clearly
		tex.filterMode = FilterMode.Point;
		// Call Apply() so it's actually uploaded to the GPU
		tex.Apply();

		// Set texture onto our matrial
		GetComponent<Renderer>().material.mainTexture = tex;

		// Pass texture pointer to the plugin
	//#if UNITY_GLES_RENDERER
		SetTextureFromUnity (tex.GetNativeTexturePtr(), tex.width, tex.height);
	/*#else
		SetTextureFromUnity (tex.GetNativeTexturePtr());
	#endif*/
	}

	public Matrix4x4 modelMatrix;

	private float[] GetRawArrayFromMatrix( Matrix4x4 matrix )
	{
		float[] rawArray = new float[16];
		for (int i = 0; i < 16; i++) {
			rawArray[i] = matrix[i];
		}
		return rawArray;
	}

	private IEnumerator CallPluginAtEndOfFrames()
	{
		while (true) {
			// Wait until all frame rendering is done
			yield return new WaitForEndOfFrame();

			// Set time for the plugin
			SetTimeFromUnity (Time.timeSinceLevelLoad);

			// Set matrices for the plugin
			SetMatricesFromUnity( GetRawArrayFromMatrix( modelMatrix ), 
			                     GetRawArrayFromMatrix( Camera.current.worldToCameraMatrix ),
			                     GetRawArrayFromMatrix( Camera.current.projectionMatrix ) );

			// Issue a plugin event with arbitrary integer identifier.
			// The plugin can distinguish between different
			// things it needs to do based on this ID.
			// For our simple plugin, it does not matter which ID we pass here.
			GL.IssuePluginEvent (1);
		}
	}
}
