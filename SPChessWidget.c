#include "SPChessWidget.h"

/**
 * The functions frees the memory of a given widget (if it not a NULL pointer).
 * We are making a NULL Safe Destroy.
 *
 * @param widget - The widget to destroy
 */
void destroyWidget(SPWidget* src) {
	if (src == NULL)
		return;

	src->destroyWidget(src);
}
