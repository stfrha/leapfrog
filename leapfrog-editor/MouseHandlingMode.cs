using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ZoomAndPan
{
    /// <summary>
    /// Defines the current state of the mouse handling logic.
    /// </summary>
    public enum MouseHandlingMode
    {
        /// <summary>
        /// Not in any special mode.
        /// </summary>
        None,

        /// <summary>
        /// The user is left-dragging objects with the mouse.
        /// </summary>
        DraggingObjects,

      /// <summary>
      /// The user is left-dragging objects with the mouse.
      /// </summary>
      Panning,
   }
}
