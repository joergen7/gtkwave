#include "gw-named-markers.h"

/**
 * GwNamedMarkers:
 *
 * A collection of named markers.
 */

static gchar *index_to_bijective_string(gsize index)
{
    GString *str = g_string_new(NULL);

    index++; // bijective values start at one

    while (index > 0) {
        index--;
        g_string_append_c(str, 'A' + index % ('Z' - 'A' + 1));

        index /= ('Z' - 'A' + 1);
    }

    g_strreverse(str->str);

    return g_string_free(str, FALSE);
}

struct _GwNamedMarkers
{
    GObject parent_instance;

    GPtrArray *markers;
};

G_DEFINE_TYPE(GwNamedMarkers, gw_named_markers, G_TYPE_OBJECT)

enum
{
    PROP_NUMBER_OF_MARKERS = 1,
    N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES];

static void gw_named_markers_dispose(GObject *object)
{
    GwNamedMarkers *self = GW_NAMED_MARKERS(object);

    g_ptr_array_remove_range(self->markers, 0, self->markers->len);

    G_OBJECT_CLASS(gw_named_markers_parent_class)->dispose(object);
}

static void gw_named_markers_finalize(GObject *object)
{
    GwNamedMarkers *self = GW_NAMED_MARKERS(object);

    g_ptr_array_free(self->markers, TRUE);

    G_OBJECT_CLASS(gw_named_markers_parent_class)->finalize(object);
}

static void gw_named_markers_set_property(GObject *object,
                                          guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec)
{
    GwNamedMarkers *self = GW_NAMED_MARKERS(object);

    switch (property_id) {
        case PROP_NUMBER_OF_MARKERS: {
            guint number_of_markers = g_value_get_uint(value);
            self->markers = g_ptr_array_new_full(number_of_markers, g_object_unref);

            for (guint i = 0; i < number_of_markers; i++) {
                gchar *name = index_to_bijective_string(i);
                GwMarker *marker = gw_marker_new(name);
                g_ptr_array_add(self->markers, marker);
                g_free(name);
            }
            break;
        }

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void gw_named_markers_class_init(GwNamedMarkersClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = gw_named_markers_dispose;
    object_class->finalize = gw_named_markers_finalize;
    object_class->set_property = gw_named_markers_set_property;

    // TODO: remove property and use dynamic array to store markers
    properties[PROP_NUMBER_OF_MARKERS] =
        g_param_spec_uint("number-of-markers",
                          "Number of markers",
                          "Number of markers",
                          1,
                          G_MAXUINT,
                          1,
                          G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}

static void gw_named_markers_init(GwNamedMarkers *self)
{
    (void)self;
}

/**
 * gw_named_markers_new:
 * @number_of_markers: the number of named markers
 *
 * Creates a new collection of named markers.
 *
 * Returns: (transfer full): a new #GwNamedMarkers
 */
GwNamedMarkers *gw_named_markers_new(gsize number_of_markers)
{
    return g_object_new(GW_TYPE_NAMED_MARKERS, "number-of-markers", number_of_markers, NULL);
}

/**
 * gw_named_markers_get_number_of_markers:
 * @self: A #GwNamedMarkers.
 *
 * Returns the number of named markers.
 *
 * Returns: The number of markers.
 */
guint gw_named_markers_get_number_of_markers(GwNamedMarkers *self)
{
    g_return_val_if_fail(GW_IS_NAMED_MARKERS(self), 0);

    return self->markers->len;
}

/**
 * gw_named_markers_get:
 * @self: A #GwNamedMarkers.
 * @guint: The index.
 *
 * Gets a named marker with the given index.
 *
 * Returns: (transfer none): The marker or %NULL if the index is out of bounds.
 */
GwMarker *gw_named_markers_get(GwNamedMarkers *self, guint index)
{
    g_return_val_if_fail(GW_IS_NAMED_MARKERS(self), NULL);

    if (index < self->markers->len) {
        return g_ptr_array_index(self->markers, index);
    } else {
        return NULL;
    }
}

/**
 * gw_named_markers_find:
 * @self: A #GwNamedMarkers.
 * @time: The time.
 *
 * Searches for a named marker at the given time.
 *
 * Returns: (transfer none): The marker or %NULL if no marker was found.
 */
GwMarker *gw_named_markers_find(GwNamedMarkers *self, GwTime time)
{
    g_return_val_if_fail(GW_IS_NAMED_MARKERS(self), NULL);

    for (guint i = 0; i < self->markers->len; i++) {
        GwMarker *marker = gw_named_markers_get(self, i);
        if (gw_marker_is_enabled(marker) && gw_marker_get_position(marker) == time) {
            return marker;
        }
    }

    return NULL;
}

/**
 * gw_named_markers_find_first_disabled:
 * @self: A #GwNamedMarkers.
 *
 * Finds the first disabled marker.
 *
 * Returns: (transfer none): The marker or %NULL if all markers are enabled.
 */
GwMarker *gw_named_markers_find_first_disabled(GwNamedMarkers *self)
{
    g_return_val_if_fail(GW_IS_NAMED_MARKERS(self), FALSE);

    for (guint i = 0; i < self->markers->len; i++) {
        GwMarker *marker = gw_named_markers_get(self, i);
        if (!gw_marker_is_enabled(marker)) {
            return marker;
        }
    }

    return NULL;
}