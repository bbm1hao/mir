/*
 * Copyright © 2012-2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MIR_TOOLKIT_MIR_SURFACE_H_
#define MIR_TOOLKIT_MIR_SURFACE_H_

#include <mir_toolkit/mir_native_buffer.h>
#include <mir_toolkit/client_types.h>
#include <mir_toolkit/common.h>
#include <mir_toolkit/mir_cursor_configuration.h>

#include <stdbool.h>

#ifdef __cplusplus
/**
 * \addtogroup mir_toolkit
 * @{
 */
extern "C" {
#endif

/**
 * Create a window specification for a normal window.
 *
 * A normal window is suitable for most application windows. It has no special semantics.
 * On a desktop shell it will typically have a title-bar, be movable, resizeable, etc.
 *
 * \param [in] connection   Connection the window will be created on
 * \param [in] width        Requested width. The server is not guaranteed to return a window of this width.
 * \param [in] height       Requested height. The server is not guaranteed to return a window of this height.
 *
 * \return                  A handle that can be passed to mir_window_create() to complete construction.
 */
MirWindowSpec* mir_create_normal_window_spec(MirConnection* connection,
                                             int width, int height);

/**
 * Create a window specification for a menu window.
 *
 * Positioning of the window is specified with respect to the parent window
 * via an adjacency rectangle. The server will attempt to choose an edge of the
 * adjacency rectangle on which to place the window taking in to account
 * screen-edge proximity or similar constraints. In addition, the server can use
 * the edge affinity hint to consider only horizontal or only vertical adjacency
 * edges in the given rectangle.
 *
 * \param [in] connection   Connection the window will be created on
 * \param [in] width        Requested width. The server is not guaranteed to
 *                          return a window of this width.
 * \param [in] height       Requested height. The server is not guaranteed to
 *                          return a window of this height.
 * \param [in] parent       A valid parent window for this menu.
 * \param [in] rect         The adjacency rectangle. The server is not
 *                          guaranteed to create a window at the requested
 *                          location.
 * \param [in] edge         The preferred edge direction to attach to. Use
 *                          mir_edge_attachment_any for no preference.
 * \return                  A handle that can be passed to mir_window_create()
 *                          to complete construction.
 */
MirWindowSpec*
mir_create_menu_window_spec(MirConnection* connection,
                            int width, int height,
                            MirWindow* parent,
                            MirRectangle* rect,
                            MirEdgeAttachment edge);

/**
 * Create a window specification for a tip window.
 *
 * Positioning of the window is specified with respect to the parent window
 * via an adjacency rectangle. The server will attempt to choose an edge of the
 * adjacency rectangle on which to place the window taking in to account
 * screen-edge proximity or similar constraints. In addition, the server can use
 * the edge affinity hint to consider only horizontal or only vertical adjacency
 * edges in the given rectangle.
 *
 * \param [in] connection   Connection the window will be created on
 * \param [in] width        Requested width. The server is not guaranteed to
 *                          return a window of this width.
 * \param [in] height       Requested height. The server is not guaranteed to
 *                          return a window of this height.
 * \param [in] parent       A valid parent window for this tip.
 * \param [in] rect         The adjacency rectangle. The server is not
 *                          guaranteed to create a window at the requested
 *                          location.
 * \param [in] edge         The preferred edge direction to attach to. Use
 *                          mir_edge_attachment_any for no preference.
 * \return                  A handle that can be passed to mir_window_create()
 *                          to complete construction.
 */
MirWindowSpec*
mir_create_tip_window_spec(MirConnection* connection,
                           int width, int height,
                           MirWindow* parent,
                           MirRectangle* rect,
                           MirEdgeAttachment edge);

/**
 * Create a window specification for a modal dialog window.
 *
 * The dialog window will have input focus; the parent can still be moved,
 * resized or hidden/minimized but no interaction is possible until the dialog
 * is dismissed.
 *
 * A dialog will typically have no close/maximize button decorations.
 *
 * During window creation, if the specified parent is another dialog window
 * the server may choose to close the specified parent in order to show this
 * new dialog window.
 *
 * \param [in] connection   Connection the window will be created on
 * \param [in] width        Requested width. The server is not guaranteed to
 *                          return a window of this width.
 * \param [in] height       Requested height. The server is not guaranteed to
 *                          return a window of this height.
 * \param [in] parent       A valid parent window.
 *
 */
MirWindowSpec*
mir_create_modal_dialog_window_spec(MirConnection* connection,
                                    int width, int height,
                                    MirWindow* parent);

/**
 * Create a window specification for a parentless dialog window.
 *
 * A parentless dialog window is similar to a normal window, but it cannot
 * be fullscreen and typically won't have any maximize/close button decorations.
 *
 * A parentless dialog is not allowed to have other dialog children. The server
 * may decide to close the parent and show the child dialog only.
 *
 * \param [in] connection   Connection the window will be created on
 * \param [in] width        Requested width. The server is not guaranteed to
 *                          return a window of this width.
 * \param [in] height       Requested height. The server is not guaranteed to
 *                          return a window of this height.
 */
MirWindowSpec*
mir_create_dialog_window_spec(MirConnection* connection,
                              int width, int height);

/**
 * Create a window specification for an input method window.
 *
 * Currently this is only appropriate for the Unity On-Screen-Keyboard.
 *
 * \param [in] connection   Connection the window will be created on
 * \param [in] width        Requested width. The server is not guaranteed to return a window of this width.
 * \param [in] height       Requested height. The server is not guaranteed to return a window of this height.
 * \return                  A handle that can be passed to mir_window_create() to complete construction.
 */
MirWindowSpec*
mir_create_input_method_window_spec(MirConnection* connection,
                                    int width, int height);

/**
 * Create a window specification.
 * This can be used with mir_window_create() to create a window or with
 * mir_window_apply_spec() to change an existing window.
 * \remark For use with mir_window_create() at least the type, width, height,
 * format and buffer_usage must be set. (And for types requiring a parent that
 * too must be set.)
 *
 * \param [in] connection   a valid mir connection
 * \return                  A handle that can ultimately be passed to
 *                          mir_window_create() or mir_window_apply_spec()
 */
MirWindowSpec* mir_create_window_spec(MirConnection* connection);

/**
 * Set the requested parent.
 *
 * \param [in] spec    Specification to mutate
 * \param [in] parent  A valid parent window.
 */
void mir_window_spec_set_parent(MirWindowSpec* spec, MirWindow* parent);

/**
 * Update a window specification with a window type.
 * This can be used with mir_window_create() to create a window or with
 * mir_window_apply_spec() to change an existing window.
 * \remark For use with mir_window_apply_spec() the shell need not support
 * arbitrary changes of type and some target types may require the setting of
 * properties such as "parent" that are not already present on the window.
 * The type transformations the server is required to support are:\n
 * regular => utility, dialog or satellite\n
 * utility => regular, dialog or satellite\n
 * dialog => regular, utility or satellite\n
 * satellite => regular, utility or dialog\n
 * popup => satellite
 *
 * \param [in] spec         Specification to mutate
 * \param [in] type         the target type of the window
 */
void mir_window_spec_set_type(MirWindowSpec* spec, MirWindowType type);

/**
 * Set the requested name.
 *
 * The window name helps the user to distinguish between multiple surfaces
 * from the same application. A typical desktop shell may use it to provide
 * text in the window titlebar, in an alt-tab switcher, or equivalent.
 *
 * \param [in] spec     Specification to mutate
 * \param [in] name     Requested name. This must be valid UTF-8.
 *                      Copied into spec; clients can free the buffer passed after this call.
 */
void mir_window_spec_set_name(MirWindowSpec* spec, char const* name);

/**
 * Set the requested width, in pixels
 *
 * \param [in] spec     Specification to mutate
 * \param [in] width    Requested width.
 *
 * \note    The requested dimensions are a hint only. The server is not
 *          guaranteed to create a window of any specific width or height.
 */
void mir_window_spec_set_width(MirWindowSpec* spec, unsigned width);

/**
 * Set the requested height, in pixels
 *
 * \param [in] spec     Specification to mutate
 * \param [in] height   Requested height.
 *
 * \note    The requested dimensions are a hint only. The server is not
 *          guaranteed to create a window of any specific width or height.
 */
void mir_window_spec_set_height(MirWindowSpec* spec, unsigned height);

/**
 * Set the requested width increment, in pixels.
 * Defines an arithmetic progression of sizes starting with min_width (if set, otherwise 0)
 * into which the window prefers to be resized.
 *
 * \param [in] spec       Specification to mutate
 * \param [in] width_inc  Requested width increment.
 *
 * \note    The requested dimensions are a hint only. The server is not guaranteed to
 *          create a window of any specific width or height.
 */
void mir_window_spec_set_width_increment(MirWindowSpec* spec, unsigned width_inc);

/**
 * Set the requested height increment, in pixels
 * Defines an arithmetic progression of sizes starting with min_height (if set, otherwise 0)
 * into which the window prefers to be resized.
 *
 * \param [in] spec       Specification to mutate
 * \param [in] height_inc Requested height increment.
 *
 * \note    The requested dimensions are a hint only. The server is not guaranteed to
 *          create a window of any specific width or height.
 */
void mir_window_spec_set_height_increment(MirWindowSpec* spec, unsigned height_inc);

/**
 * Set the minimum width, in pixels
 *
 * \param [in] spec       Specification to mutate
 * \param [in] min_width  Minimum width.
 *
 * \note    The requested dimensions are a hint only. The server is not guaranteed to create a
 *          window of any specific width or height.
 */
void mir_window_spec_set_min_width(MirWindowSpec* spec, unsigned min_width);

/**
 * Set the minimum height, in pixels
 *
 * \param [in] spec       Specification to mutate
 * \param [in] min_height Minimum height.
 *
 * \note    The requested dimensions are a hint only. The server is not guaranteed to create a
 *          window of any specific width or height.
 */
void mir_window_spec_set_min_height(MirWindowSpec* spec, unsigned min_height);

/**
 * Set the maximum width, in pixels
 *
 * \param [in] spec       Specification to mutate
 * \param [in] max_width  maximum width.
 *
 * \note    The requested dimensions are a hint only. The server is not guaranteed to create a
 *          window of any specific width or height.
 */
void mir_window_spec_set_max_width(MirWindowSpec* spec, unsigned max_width);

/**
 * Set the maximum height, in pixels
 *
 * \param [in] spec       Specification to mutate
 * \param [in] max_height maximum height.
 *
 * \note    The requested dimensions are a hint only. The server is not guaranteed to create a
 *          window of any specific width or height.
 */
void mir_window_spec_set_max_height(MirWindowSpec* spec, unsigned max_height);

/**
 * Set the minimum aspect ratio. This is the minimum ratio of window width to height.
 * It is independent of orientation changes and/or preferences.
 *
 * \param [in] spec     Specification to mutate
 * \param [in] width    numerator
 * \param [in] height   denominator
 *
 * \note    The requested aspect ratio is a hint only. The server is not guaranteed
 *          to create a window of any specific aspect.
 */
void mir_window_spec_set_min_aspect_ratio(MirWindowSpec* spec, unsigned width, unsigned height);

/**
 * Set the maximum aspect ratio. This is the maximum ratio of window width to height.
 * It is independent of orientation changes and/or preferences.
 *
 * \param [in] spec     Specification to mutate
 * \param [in] width    numerator
 * \param [in] height   denominator
 *
 * \note    The requested aspect ratio is a hint only. The server is not guaranteed
 *          to create a window of any specific aspect.
 */
void mir_window_spec_set_max_aspect_ratio(MirWindowSpec* spec, unsigned width, unsigned height);

/**
 * \param [in] spec         Specification to mutate
 * \param [in] output_id    ID of output to place window on. From MirDisplayOutput.output_id
 *
 * \note    If this call returns %true then a valid window returned from mir_window_create() is
 *          guaranteed to be fullscreen on the specified output. An invalid window is returned
 *          if the server unable to, or policy prevents it from, honouring this request.
 */
void mir_window_spec_set_fullscreen_on_output(MirWindowSpec* spec, uint32_t output_id);

/**
 * Set the requested preferred orientation mode.
 * \param [in] spec    Specification to mutate
 * \param [in] mode    Requested preferred orientation
 *
 * \note    If the server is unable to create a window with the preferred orientation at
 *          the point mir_window_create() is called it will instead return an invalid window.
 */
void mir_window_spec_set_preferred_orientation(MirWindowSpec* spec, MirOrientationMode mode);

/**
 * Request that the created window be attached to a window of a different client.
 *
 * This is restricted to input methods, which need to attach their suggestion window
 * to text entry widgets of other processes.
 *
 * \param [in] spec             Specification to mutate
 * \param [in] parent           A MirPersistentId reference to the parent window
 * \param [in] attachment_rect  A rectangle specifying the region (in parent window coordinates)
 *                              that the created window should be attached to.
 * \param [in] edge             The preferred edge direction to attach to. Use
 *                              mir_edge_attachment_any for no preference.
 * \return                      False if the operation is invalid for this window type.
 *
 * \note    If the parent window becomes invalid before mir_window_create() is processed,
 *          it will return an invalid window. If the parent window is valid at the time
 *          mir_window_create() is called but is later closed, this window will also receive
 *          a close event.
 */
bool mir_window_spec_attach_to_foreign_parent(MirWindowSpec* spec,
                                              MirPersistentId* parent,
                                              MirRectangle* attachment_rect,
                                              MirEdgeAttachment edge);

/**
 * Set the requested state.
 * \param [in] spec    Specification to mutate
 * \param [in] state   Requested state
 *
 * \note    If the server is unable to create a window with the requested state at
 *          the point mir_window_create() is called it will instead return an invalid window.
 */
void mir_window_spec_set_state(MirWindowSpec* spec, MirWindowState state);

/**
 * Set a collection of input rectangles associated with the spec.
 * Rectangles are specified as a list of regions relative to the top left
 * of the specified window. If the server applies this specification
 * to a window input which would normally go to the window but is not
 * contained within any of the input rectangles instead passes
 * on to the next client.
 *
 * \param [in] spec The spec to accumulate the request in.
 * \param [in] rectangles An array of MirRectangles specifying the input shape.
 * \param [in] n_rects The number of elements in the rectangles array.
 */
void mir_window_spec_set_input_shape(MirWindowSpec* spec,
                                     MirRectangle const *rectangles,
                                     size_t n_rects);

/**
 * Set the event handler to be called when events arrive for a window.
 *   \warning event_handler could be called from another thread. You must do
 *            any locking appropriate to protect your data accessed in the
 *            callback. There is also a chance that different events will be
 *            called back in different threads, for the same window,
 *            simultaneously.
 * \param [in] spec       The spec to accumulate the request in.
 * \param [in] callback   The callback function
 * \param [in] context    Additional argument to be passed to callback
 */
void mir_window_spec_set_event_handler(MirWindowSpec* spec,
                                       mir_window_event_callback callback,
                                       void* context);

/**
 * Ask the shell to customize "chrome" for this window.
 * For example, on the phone hide indicators when this window is active.
 *
 * \param [in] spec The spec to accumulate the request in.
 * \param [in] style The requested level of "chrome"
 */
void mir_window_spec_set_shell_chrome(MirWindowSpec* spec, MirShellChrome style);

/**
 * Attempts to set the pointer confinement spec for this window
 *
 * This will request the window manager to confine the pointer to the surfaces region.
 *
 * \param [in] spec  The spec to accumulate the request in.
 * \param [in] state The state you would like the pointer confinement to be in.
 */
void mir_window_spec_set_pointer_confinement(MirWindowSpec* spec, MirPointerConfinementState state);

/**
 * Set the window placement on the spec.
 *
 * \param [in] spec             the spec to update
 * \param [in] rect             the destination rectangle to align with
 * \param [in] rect_gravity     the point on \p rect to align with
 * \param [in] surface_gravity  the point on the window to align with
 * \param [in] placement_hints  positioning hints to use when limited on space
 * \param [in] offset_dx        horizontal offset to shift w.r.t. \p rect
 * \param [in] offset_dy        vertical offset to shift w.r.t. \p rect
 *
 * Moves a window to \p rect, aligning their reference points.
 *
 * \p rect is relative to the top-left corner of the parent window.
 * \p rect_gravity and \p surface_gravity determine the points on \p rect and
 * the window to pin together. \p rect's alignment point can be offset by
 * \p offset_dx and \p offset_dy, which is equivalent to offsetting the
 * position of the window.
 *
 * \p placement_hints determine how the window should be positioned in the case
 * that the window would fall off-screen if placed in its ideal position.
 * See \ref MirPlacementHints for details.
 */
void mir_window_spec_set_placement(MirWindowSpec*      spec,
                                   const MirRectangle* rect,
                                   MirPlacementGravity rect_gravity,
                                   MirPlacementGravity window_gravity,
                                   MirPlacementHints   placement_hints,
                                   int                 offset_dx,
                                   int                 offset_dy);

/**
 * Set the name for the cursor from the system cursor theme.
 * \param [in] spec             The spec
 * \param [in] name             The name, or "" to reset to default
 */
void mir_window_spec_set_cursor_name(MirWindowSpec* spec, char const* name);

/**
 * \note To be deprecated soon. Only for enabling other deprecations.
 *
 * Set the requested pixel format.
 * \param [in] spec     Specification to mutate
 * \param [in] format   Requested pixel format
 *
 * \note    If this call returns %true then the server is guaranteed to honour this request.
 *          If the server is unable to create a window with this pixel format at
 *          the point mir_window_create() is called it will instead return an invalid window.
 */
void mir_window_spec_set_pixel_format(MirWindowSpec* spec, MirPixelFormat format);

/**
 * \note To be deprecated soon. Only for enabling other deprecations.
 *
 * Set the requested buffer usage.
 * \param [in] spec     Specification to mutate
 * \param [in] usage    Requested buffer usage
 *
 * \note    If this call returns %true then the server is guaranteed to honour this request.
 *          If the server is unable to create a window with this buffer usage at
 *          the point mir_window_create() is called it will instead return an invalid window.
 */
void mir_window_spec_set_buffer_usage(MirWindowSpec* spec, MirBufferUsage usage);

/**
 * \note To be deprecated soon. Waiting for mir_window_spec_set_render_surfaces() to land.
 *
 * Set the streams associated with the spec.
 * streams[0] is the bottom-most stream, and streams[size-1] is the topmost.
 * On application of the spec, a stream that is present in the window,
 * but is not in the list will be disassociated from the window.
 * On application of the spec, a stream that is not present in the window,
 * but is in the list will be associated with the window.
 * Streams set a displacement from the top-left corner of the window.
 *
 * \warning disassociating streams from the window will not release() them.
 * \warning It is wiser to arrange the streams within the bounds of the
 *          window the spec is applied to. Shells can define their own
 *          behavior as to what happens to an out-of-bound stream.
 *
 * \param [in] spec        The spec to accumulate the request in.
 * \param [in] streams     An array of non-null streams info.
 * \param [in] num_streams The number of elements in the streams array.
 */
void mir_window_spec_set_streams(MirWindowSpec* spec,
                                 MirBufferStreamInfo* streams,
                                 unsigned int num_streams);

/**
 * Release the resources held by a MirWindowSpec.
 *
 * \param [in] spec     Specification to release
 */
void mir_window_spec_release(MirWindowSpec* spec);

/**
 * Request changes to the specification of a window. The server will decide
 * whether and how the request can be honoured.
 *
 *   \param [in] window  The window to mutate
 *   \param [in] spec    Spec with the requested changes applied
 */
void mir_window_apply_spec(MirWindow* window, MirWindowSpec* spec);

/**
 * Create a window from a given specification
 *
 *
 * \param [in] requested_specification  Specification of the attributes for the created window
 * \param [in] callback                 Callback function to be invoked when creation is complete
 * \param [in, out] context             User data passed to callback function.
 *                                      This callback is guaranteed to be called, and called with a
 *                                      non-null MirWindow*, but the window may be invalid in
 *                                      case of an error.
 */
void mir_window_create(MirWindowSpec* requested_specification,
                       mir_window_callback callback, void* context);

/**
 * Create a window from a given specification and wait for the result.
 * \param [in] requested_specification  Specification of the attributes for the created window
 * \return                              The new window. This is guaranteed non-null, but may be invalid
 *                                      in the case of error.
 */
MirWindow* mir_window_create_sync(MirWindowSpec* requested_specification);

/**
 * Release the supplied window and any associated buffer.
 *
 *   \warning callback could be called from another thread. You must do any
 *            locking appropriate to protect your data accessed in the
 *            callback.
 *   \param [in] window       The window
 *   \param [in] callback     Callback function to be invoked when the request
 *                            completes
 *   \param [in,out] context  User data passed to the callback function
 */
void mir_window_release(
    MirWindow* window,
    mir_window_callback callback,
    void *context);

/**
 * Release the specified window like in mir_window_release(), but also wait
 * for the operation to complete.
 *   \param [in] window  The window to be released
 */
void mir_window_release_sync(MirWindow* window);

/**
 * Test for a valid window
 *   \param [in] window   The window
 *   \return              True if the supplied window is valid, or
 *                        false otherwise.
 */
bool mir_window_is_valid(MirWindow* window);

/**
 * Set the event handler to be called when events arrive for a window.
 *   \warning event_handler could be called from another thread. You must do
 *            any locking appropriate to protect your data accessed in the
 *            callback. There is also a chance that different events will be
 *            called back in different threads, for the same window,
 *            simultaneously.
 *   \param [in] window         The window
 *   \param [in] callback       The callback function
 *   \param [in] context        Additional argument to be passed to callback
 */
void mir_window_set_event_handler(MirWindow* window,
                                  mir_window_event_callback callback,
                                  void* context);

/**
 * Retrieve the primary MirBufferStream associated with a window (to advance buffers,
 * obtain EGLNativeWindow, etc...)
 *
 *   \deprecated Users should use mir_window_spec_set_streams() to arrange
 *               the content of a window, instead of relying on a stream
 *               being created by default.
 *   \warning If the window was created with, or modified to have a
 *            MirWindowSpec containing streams added through
 *            mir_window_spec_set_streams(), the default stream will
 *            be removed, and this function will return NULL.
 *   \param[in] window The window
 */
MirBufferStream* mir_window_get_buffer_stream(MirWindow* window);

/**
 * Retrieve a text description of the error. The returned string is owned by
 * the library and remains valid until the window or the associated
 * connection has been released.
 *   \param [in] window  The window
 *   \return              A text description of any error resulting in an
 *                        invalid window, or the empty string "" if the
 *                        connection is valid.
 */
char const* mir_window_get_error_message(MirWindow* window);

/**
 * Get a window's parameters.
 *   \pre                     The window is valid
 *   \param [in]  window      The window
 *   \param [out] parameters  Structure to be populated
 */
void mir_window_get_parameters(MirWindow* window, MirWindowParameters* parameters);

/**
 * Get the orientation of a window.
 *   \param [in] window  The window to query
 *   \return              The orientation of the window
 */
MirOrientation mir_window_get_orientation(MirWindow* window);

/**
 * Attempts to raise the window to the front.
 *
 * \param [in] window The window to raise
 * \param [in] cookie  A cookie instance obtained from an input event.
 *                     An invalid cookie will terminate the client connection.
 */
void mir_window_raise(MirWindow* window, MirCookie const* cookie);

/**
 * Get the type (purpose) of a window.
 *   \param [in] window  The window to query
 *   \return              The type of the window
 */
MirWindowType mir_window_get_type(MirWindow* window);

// Functions in this pragma section are to be deprecated
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

MirSurfaceSpec* mir_connection_create_spec_for_normal_surface(MirConnection* connection,
                                                              int width, int height,
                                                              MirPixelFormat format);
//__attribute__((deprecated("Use mir_create_normal_window_spec() instead")));

MirSurfaceSpec*
mir_connection_create_spec_for_menu(MirConnection* connection,
                                    int width,
                                    int height,
                                    MirPixelFormat format,
                                    MirSurface* parent,
                                    MirRectangle* rect,
                                    MirEdgeAttachment edge);
//__attribute__((deprecated("Use mir_specify_menu() instead")));

MirSurfaceSpec*
mir_connection_create_spec_for_tooltip(MirConnection* connection,
                                       int width, int height,
                                       MirPixelFormat format,
                                       MirSurface* parent,
                                       MirRectangle* zone);
//__attribute__((deprecated("Use mir_create_tip_window_spec() instead")));

MirSurfaceSpec*
mir_connection_create_spec_for_tip(MirConnection* connection,
                                   int width, int height,
                                   MirPixelFormat format,
                                   MirSurface* parent,
                                   MirRectangle* rect,
                                   MirEdgeAttachment edge);
//__attribute__((deprecated("Use mir_create_tip_window_spec() instead")));

MirSurfaceSpec*
mir_connection_create_spec_for_modal_dialog(MirConnection* connection,
                                            int width, int height,
                                            MirPixelFormat format,
                                            MirSurface* parent);
//__attribute__((deprecated("Use mir_create_modal_dialog_window_spec() instead")));

MirSurfaceSpec*
mir_connection_create_spec_for_dialog(MirConnection* connection,
                                      int width, int height,
                                      MirPixelFormat format);
//__attribute__((deprecated("Use mir_create_dialog_window_spec() instead")));

MirSurfaceSpec* mir_create_surface_spec(MirConnection* connection);
//__attribute__((deprecated("Use mir_create_window_spec() instead")));

MirSurfaceSpec*
mir_connection_create_spec_for_changes(MirConnection* connection);
//__attribute__((deprecated("Use mir_create_window_spec() instead")));

void mir_surface_spec_set_parent(MirSurfaceSpec* spec, MirSurface* parent);
//__attribute__((deprecated("Use mir_window_spec_set_parent() instead")));

/**
 *\deprecated This will soon be a property of the backing content.
 *
 * Query the swapinterval that the surface is operating with.
 * The default interval is 1.
 *   \param [in] surface  The surface to operate on
 *   \return              The swapinterval value that the client is operating with.
 *                        Returns -1 if surface is invalid, or if the default stream
 *                        was removed by use of mir_window_spec_set_streams().
 */
int mir_surface_get_swapinterval(MirSurface* surface)
__attribute__((deprecated("This will soon be a property of the backing content")));

void mir_surface_spec_set_type(MirSurfaceSpec* spec, MirSurfaceType type);
//__attribute__((deprecated("use mir_window_spec_set_type() instead")));

void mir_surface_spec_set_name(MirSurfaceSpec* spec, char const* name);
//__attribute__((deprecated("use mir_window_spec_set_name() instead")));

void mir_surface_spec_set_width(MirSurfaceSpec* spec, unsigned width);
//__attribute__((deprecated("use mir_window_spec_set_width() instead")));

void mir_surface_spec_set_height(MirSurfaceSpec* spec, unsigned height);
//__attribute__((deprecated("use mir_window_spec_set_height() instead")));

void mir_surface_spec_set_width_increment(MirSurfaceSpec* spec, unsigned width_inc);
//__attribute__((deprecated("use mir_window_spec_set_width_increment() instead")));

void mir_surface_spec_set_height_increment(MirSurfaceSpec* spec, unsigned height_inc);
//__attribute__((deprecated("use mir_window_spec_set_height_increment() instead")));

void mir_surface_spec_set_min_width(MirSurfaceSpec* spec, unsigned min_width);
//__attribute__((deprecated("use mir_window_spec_set_min_width() instead")));

void mir_surface_spec_set_min_height(MirSurfaceSpec* spec, unsigned min_height);
//__attribute__((deprecated("use mir_window_spec_set_min_height() instead")));

void mir_surface_spec_set_max_width(MirSurfaceSpec* spec, unsigned max_width);
//__attribute__((deprecated("use mir_window_spec_set_max_width() instead")));

void mir_surface_spec_set_max_height(MirSurfaceSpec* spec, unsigned max_height);
//__attribute__((deprecated("use mir_window_spec_set_max_height() instead")));

void mir_surface_spec_set_min_aspect_ratio(MirSurfaceSpec* spec, unsigned width, unsigned height);
//__attribute__((deprecated("use mir_window_spec_set_min_aspect_ratio() instead")));

void mir_surface_spec_set_max_aspect_ratio(MirSurfaceSpec* spec, unsigned width, unsigned height);
//__attribute__((deprecated("use mir_window_spec_set_max_aspect_ratio() instead")));

void mir_surface_spec_set_fullscreen_on_output(MirSurfaceSpec* spec, uint32_t output_id);
//__attribute__((deprecated("use mir_window_spec_set_fullscreen_on_output() instead")));

void mir_surface_spec_set_preferred_orientation(MirSurfaceSpec* spec, MirOrientationMode mode);
//__attribute__((deprecated("use mir_window_spec_set_preferred_orientation() instead")));

bool mir_surface_spec_attach_to_foreign_parent(MirSurfaceSpec* spec,
                                               MirPersistentId* parent,
                                               MirRectangle* attachment_rect,
                                               MirEdgeAttachment edge);
//__attribute__((deprecated("use mir_window_spec_attach_to_foreign_parent() instead")));

void mir_surface_spec_set_state(MirSurfaceSpec* spec, MirSurfaceState state);
//__attribute__((deprecated("use mir_window_spec_set_state() instead")));

void mir_surface_spec_release(MirSurfaceSpec* spec);
//__attribute__((deprecated("use mir_window_spec_release() instead")));

void mir_surface_spec_set_input_shape(MirSurfaceSpec* spec,
                                      MirRectangle const *rectangles,
                                      size_t n_rects);
//__attribute__((deprecated("use mir_window_spec_set_input_shape() instead")));

void mir_surface_spec_set_event_handler(MirSurfaceSpec* spec,
                                        mir_surface_event_callback callback,
                                        void* context);
//__attribute__((deprecated("use mir_window_spec_set_event_handler() instead")));

void mir_surface_spec_set_shell_chrome(MirSurfaceSpec* spec, MirShellChrome style);
//__attribute__((deprecated("use mir_window_spec_set_shell_chrome() instead")));

void mir_surface_spec_set_pointer_confinement(MirSurfaceSpec* spec, MirPointerConfinementState state);
//__attribute__((deprecated("use mir_window_spec_set_pointer_confinement() instead")));

void mir_surface_spec_set_placement(MirSurfaceSpec*     spec,
                                    const MirRectangle* rect,
                                    MirPlacementGravity rect_gravity,
                                    MirPlacementGravity window_gravity,
                                    MirPlacementHints   placement_hints,
                                    int                 offset_dx,
                                    int                 offset_dy);
//__attribute__((deprecated("use mir_window_spec_set_placement() instead")));

MirSurfaceSpec* mir_connection_create_spec_for_input_method(MirConnection* connection,
                                                            int width, int height,
                                                            MirPixelFormat format);
//__attribute__((deprecated("use mir_create_input_method_window_spec() instead")));

void mir_surface_spec_set_pixel_format(MirSurfaceSpec* spec, MirPixelFormat format);
//__attribute__((deprecated("use mir_window_spec_set_pixel_format() instead")));

void mir_surface_spec_set_buffer_usage(MirSurfaceSpec* spec, MirBufferUsage usage);
//__attribute__((deprecated("use mir_window_spec_set_buffer_usage() instead")));

void mir_surface_spec_set_streams(MirSurfaceSpec* spec,
                                  MirBufferStreamInfo* streams,
                                  unsigned int num_streams);
//__attribute__((deprecated("use mir_window_spec_set_streams() instead")));

void mir_surface_apply_spec(MirSurface* surface, MirSurfaceSpec* spec);
//__attribute__((deprecated("use mir_window_apply_spec() instead")));

bool mir_surface_is_valid(MirSurface *surface);
//__attribute__((deprecated("use mir_window_is_valid() instead")));

MirWaitHandle* mir_surface_create(MirSurfaceSpec* requested_specification,
                                  mir_surface_callback callback, void* context);
//__attribute__((deprecated("use mir_window_create() instead")));

MirSurface* mir_surface_create_sync(MirSurfaceSpec* requested_specification);
//__attribute__((deprecated("use mir_window_create_sync() instead")));

MirWaitHandle *mir_surface_release(
    MirSurface *surface,
    mir_surface_callback callback,
    void *context);
//__attribute__((deprecated("use mir_window_release() instead")));

void mir_surface_release_sync(MirSurface *surface);
//__attribute__((deprecated("use mir_window_release_sync() instead")));

void mir_surface_set_event_handler(MirSurface *surface,
                                   mir_surface_event_callback callback,
                                   void* context);
//__attribute__((deprecated("use mir_window_set_event_handler() instead")));

MirBufferStream* mir_surface_get_buffer_stream(MirSurface *surface);
//__attribute__((deprecated("use mir_window_get_buffer_stream() instead")));

char const* mir_surface_get_error_message(MirSurface *surface);
//__attribute__((deprecated("use mir_window_get_error_message() instead")));

void mir_surface_get_parameters(MirSurface *surface, MirSurfaceParameters *parameters);
//__attribute__((deprecated("use mir_window_get_parameters() instead")));

MirSurfaceType mir_surface_get_type(MirSurface* surface);
//__attribute__((deprecated("use mir_window_get_type() instead")));

/**
 * Change the state of a surface.
 *   \param [in] surface  The surface to operate on
 *   \param [in] state    The new state of the surface
 *   \return              A wait handle that can be passed to mir_wait_for
 */
MirWaitHandle* mir_surface_set_state(MirSurface *surface,
                                     MirSurfaceState state);

/**
 * Get the current state of a surface.
 *   \param [in] surface  The surface to query
 *   \return              The state of the surface
 */
MirSurfaceState mir_surface_get_state(MirSurface *surface);

/**
 * Set the swapinterval for the default stream.
 *   \warning EGL users should use eglSwapInterval directly.
 *   \warning Only swapinterval of 0 or 1 is supported.
 *   \warning If the surface was created with, or modified to have a
 *            MirSurfaceSpec containing streams added through
 *            mir_window_spec_set_streams(), the default stream will
 *            be removed, and this function will return NULL.
 *   \param [in] surface  The surface to operate on
 *   \param [in] interval The number of vblank signals that
 *                        mir_surface_swap_buffers will wait for
 *   \return              A wait handle that can be passed to mir_wait_for,
 *                        or NULL if the interval could not be supported
 */
MirWaitHandle* mir_surface_set_swapinterval(MirSurface* surface, int interval)
__attribute__((deprecated("Swap interval should be set on the backing content")));

/**
 * Query the DPI value of the surface (dots per inch). This will vary depending
 * on the physical display configuration and where the surface is within it.
 *   \return  The DPI of the surface, or zero if unknown.
 */
int mir_surface_get_dpi(MirSurface* surface);
    
/**
 * Query the focus state for a surface.
 *   \param [in] surface The surface to operate on
 *   \return             The focus state of said surface
 */
MirSurfaceFocusState mir_surface_get_focus(MirSurface *surface);

/**
 * Query the visibility state for a surface.
 *   \param [in] surface The surface to operate on
 *   \return             The visibility state of said surface
 */
MirSurfaceVisibility mir_surface_get_visibility(MirSurface *surface);

/**
 * Choose the cursor state for a surface: whether a cursor is shown, 
 * and which cursor if so.
 *    \param [in] surface    The surface to operate on
 *    \param [in] parameters The configuration parameters obtained
 *                           from mir_cursor* family of functions.
 *    \return                A wait handle that can be passed to mir_wait_for,
 *                           or NULL if parameters is invalid.
 *
 */
MirWaitHandle* mir_surface_configure_cursor(MirSurface *surface, MirCursorConfiguration const* parameters);

MirOrientation mir_surface_get_orientation(MirSurface *surface);
//__attribute__((deprecated("use mir_window_get_orientation() instead")));

/**
 * Request to set the preferred orientations of a surface.
 * The request may be rejected by the server; to check wait on the
 * result and check the applied value using mir_surface_get_preferred_orientation
 *   \param [in] surface     The surface to operate on
 *   \param [in] orientation The preferred orientation modes
 *   \return                 A wait handle that can be passed to mir_wait_for
 */
MirWaitHandle* mir_surface_set_preferred_orientation(MirSurface *surface, MirOrientationMode orientation);

/**
 * Get the preferred orientation modes of a surface.
 *   \param [in] surface  The surface to query
 *   \return              The preferred orientation modes
 */
MirOrientationMode mir_surface_get_preferred_orientation(MirSurface *surface);

/**
 * \brief Request an ID for the surface that can be shared cross-process and
 *        across restarts.
 *
 * This call acquires a MirPersistentId for this MirSurface. This MirPersistentId
 * can be serialized to a string, stored or sent to another process, and then
 * later deserialized to refer to the same surface.
 *
 * \param [in]     surface   The surface to acquire a persistent reference to.
 * \param [in]     callback  Callback to invoke when the request completes.
 * \param [in,out] context   User data passed to completion callback.
 * \return A MirWaitHandle that can be used in mir_wait_for to await completion.
 */
MirWaitHandle* mir_surface_request_persistent_id(MirSurface* surface, mir_surface_id_callback callback, void* context);

/**
 * \brief Request a persistent ID for a surface and wait for the result
 * \param [in] surface  The surface to acquire a persistent ID for.
 * \return A MirPersistentId. This MirPersistentId is owned by the calling code, and must
 *         be freed with a call to mir_persistent_id_release()
 */
MirPersistentId* mir_surface_request_persistent_id_sync(MirSurface *surface);

void mir_surface_raise(MirSurface* surface, MirCookie const* cookie);
//__attribute__((deprecated("use mir_window_raise() instead")));

//#pragma GCC diagnostic pop

/**
 * \brief Check the validity of a MirPersistentId
 * \param [in] id  The MirPersistentId
 * \return True iff the MirPersistentId contains a valid ID value.
 *
 * \note This does not guarantee that the ID refers to a currently valid object.
 */
bool mir_persistent_id_is_valid(MirPersistentId* id);

/**
 * \brief Free a MirPersistentId
 * \param [in] id  The MirPersistentId to free
 * \note This frees only the client-side representation; it has no effect on the
 *       object referred to by \arg id.
 */
void mir_persistent_id_release(MirPersistentId* id);

/**
 * \brief Get a string representation of a MirSurfaceId
 * \param [in] id  The ID to serialise
 * \return A string representation of id. This string is owned by the MirSurfaceId,
 *         and must not be freed by the caller.
 *
 * \see mir_surface_id_from_string
 */
char const* mir_persistent_id_as_string(MirPersistentId* id);

/**
 * \brief Deserialise a string representation of a MirSurfaceId
 * \param [in] string_representation  Serialised representation of the ID
 * \return The deserialised MirSurfaceId
 */
MirPersistentId* mir_persistent_id_from_string(char const* string_representation);

#ifdef __cplusplus
}
/**@}*/
#endif

#endif /* MIR_TOOLKIT_MIR_SURFACE_H_ */
