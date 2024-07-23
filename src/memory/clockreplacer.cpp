#include "clockreplacer.h"

Memory::ClockReplacer::ClockReplacer(int num_frames) : numFrames(num_frames), referenceBits(num_frames, false),
    frames(num_frames, -1), clockHand(0) {}

Memory::ClockReplacer::~ClockReplacer() = default;

void Memory::ClockReplacer::advanceClock()
{
    clockHand = (clockHand + 1) % numFrames;
}

bool Memory::ClockReplacer::victim(frame_id_t *frame_id)
{
    // if no victim page exists
    if (size() == 0) {
        return false;
    }

    while (true) {
        auto &frame = frames[clockHand];
        // if refBit == 0, pick this frame as victim
        if (frames[clockHand] != -1 && !referenceBits[clockHand]) {
            // set victim's frame id
            *frame_id = frames[clockHand];
            // remove it from the list & map
            frames[clockHand] = -1;
            frameMap.remove(*frame_id);
            // advance clock one position
            advanceClock();
            return true;
        }
        // else, give it a second chance
        referenceBits[clockHand] = false;
        // increment current clockHand
        advanceClock();
    }
}

void Memory::ClockReplacer::pin(frame_id_t frame_id)
{
    // when the frame is not in the frame victim deque, then
    // there's nothing to do
    auto it = frameMap.find(frame_id);
    if (it == frameMap.end()) {
        return;
    }

    // the page is pinned (in use), so it cannot be evicted (chosen as a victim)
    // remove it from the victim list
    auto index = it.value();
    frames[index] = -1;
    frameMap.remove(frame_id);
}

void Memory::ClockReplacer::unpin(frame_id_t frame_id)
{
    // we do not reinsert a victim frame if found previously
    if (frameMap.contains(frame_id)) {
        return;
    }

    // store current clock position
    size_t start_hand = clockHand;
    // scroll through the circular list, to find an empty slot
    while (frames[clockHand] != -1) {
        advanceClock();
        // if an empty slot wasn't found
        if (clockHand == start_hand) {
            return;
        }
    }
    // empty slot found, place the frame
    frames[clockHand] = frame_id;
    // Update frameMap
    frameMap[frame_id] = clockHand;
    // set the refBit
    referenceBits[clockHand] = true;
    // move the clock to the next position
    advanceClock();
}

size_t Memory::ClockReplacer::size()
{
    size_t count = 0;
    for (const auto &frame : frames) {
        if (frame != -1) {
            count++;
        }
    }
    return count;
}
