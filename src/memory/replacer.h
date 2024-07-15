#ifndef REPLACER_H
#define REPLACER_H

#include "frame.h"

namespace Memory
{
    // Replacer Interface
    class Replacer
    {
    public:
        /**
         * @brief Constructs a Replacer
         */
        Replacer() = default;

        /**
         * @brief Destroys a Replacer
         */
        virtual ~Replacer() = default;

        /**
         * Remove the victim frame as defined by the replacement policy.
         * @param[out] frame_id id of frame that was removed, nullptr if no victim was found
         * @return true if a victim frame was found, false otherwise
         */
        virtual auto victim(frame_id_t* frame_id) -> bool = 0;

        /**
         * Pins a frame, indicating that it should not be victimized until it is unpinned.
         * @param frame_id the id of the frame to pin
         */
        virtual void pin(frame_id_t frame_id) = 0;

        /**
         * Unpins a frame, indicating that it can now be victimized.
         * @param frame_id the id of the frame to unpin
         */
        virtual void unpin(frame_id_t frame_id) = 0;

        /** @return the number of elements in the replacer that can be victimized */
        virtual auto size() -> size_t = 0;
    };
}


#endif // REPLACER_H
