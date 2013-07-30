This program allows the user to input a set of polygons, scan converts those polygons onto the screen, allows the user to specify a clipping rectangle and displays the clipped polygons. 


These are the salient features of the program:

1. Allows the user to specify the polygons using the mouse. The user will press the left mouse button to specify a vertex. Each button press will be the next vertex (in order) for the polygon. The final point of the polygon will be specified by pressing the right mouse button. 
    So the user can specify a triangle with two left clicks and one right click. The user may define up to 10 polygons, each with at most 10 vertices (before clipping).

2. Lets the user enter clipping mode by pressing 'c'. Once this key has been pressed, no more polygons can be entered.It allows the user to specify the clipping rectangle by clicking and dragging the mouse. The point the user clicks with the left mouse button will be one corner of the rectangle and the point the mouse is released will be the other. The clipping region will be highlighted as the user drags the mouse (draw the four edges of the rectangle) with the polygons the user entered still being shown.

3. It allows the user to change the clip region as many times as desired. In each case, the original polygons are used for clipping and not the already clipped polygons.