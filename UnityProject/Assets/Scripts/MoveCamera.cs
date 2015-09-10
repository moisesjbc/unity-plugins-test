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

		if (Input.touchCount > 0) {
			Touch touch = Input.GetTouch(0);
			TouchPhase phase = touch.phase;

			if( phase == TouchPhase.Moved ){
				transform.Translate( 0.25f * touch.deltaPosition.y * Vector3.forward );
				transform.Translate( 0.25f * touch.deltaPosition.x * Vector3.right );
			}
		}
	}
}