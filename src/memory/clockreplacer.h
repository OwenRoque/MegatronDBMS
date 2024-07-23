#ifndef CLOCKREPLACER_H
#define CLOCKREPLACER_H

#include "replacer.h"

namespace Memory
{
    class ClockReplacer : public Replacer
    {
    public:
        /**
         * @brief Create a new ClockReplacer.
         * @param num_frames the maximum number of frames/pages the ClockReplacer will be required to store
         */
        ClockReplacer(int num_frames);

        /**
         * @brief Destroys the ClockReplacer.
         */
        ~ClockReplacer() override;

        /**
         * @brief Remove the victim frame as defined by the replacement policy.
         * @param[out] frame_id id of frame that was removed, nullptr if no victim was found
         * @return true if a victim frame was found, false otherwise
         */
        auto victim(frame_id_t *frame_id) -> bool override;

        /**
         * @brief Pins a frame, indicating that it should not be victimized until it is unpinned.
         * @param frame_id the id of the frame to pin
         */
        void pin(frame_id_t frame_id) override;

        /**
         * @brief Unpins a frame, indicating that it can now be victimized.
         * @param frame_id the id of the frame to unpin
         */
        void unpin(frame_id_t frame_id) override;

        /** @return the number of elements in the replacer that can be victimized */
        auto size() -> size_t override;

    private:
        // maximum capacity of frame pool, same as pool size
        size_t numFrames;
        // list of 'victimizable' frames
        QList<frame_id_t> frames;
        // reference bits of the frames
        QList<bool> referenceBits;
        // to fetch the address of a key in the list quickly (find() takes O(N))
        QHash<frame_id_t, size_t> frameMap;
        // clock hand
        size_t clockHand;
        // move the hand to the next position
        void advanceClock();

    };
}

#endif // CLOCKREPLACER_H
