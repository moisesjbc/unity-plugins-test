using UnityEngine;
using System.Collections;

public class MoveCamera : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		const float step = 0.01f;
		if( Input.GetKey( KeyCode.UpArrow ) ){
			transform.Translate( step * Vector3.forward );
		}
		if( Input.GetKey( KeyCode.DownArrow ) )
		{
			transform.Translate( -step * Vector3.forward );
		}
	}
}
