#define _XOPEN_SOURCE_EXTENDED
#include <SDL2/SDL.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include <ncurses.h>

#include "SubprocessFunctions.h"
#include "MusicFunctions.h"

#define SAMPLE_RATE 44100
#define AMPLITUDE 5000
#define MAX_LINES 20000
// Generate a square wave

void *Music(void *TSharedData){

    struct D2Sshared *SharedData = (struct D2Sshared *)TSharedData;

    FILE *file = fopen("music.txt", "r");

    double time[MAX_LINES];
    double notes[MAX_LINES];
    double durations[MAX_LINES];
    int count = 0;

    // Read time and frequency from file
    while (count < MAX_LINES && fscanf(file, "%lf %lf", &time[count], &notes[count]) == 2) {
        count++;
    }

    fclose(file);

    // Calculate time
    for (int i = 0; i < count - 1; i++) {
        durations[i] = time[i + 1] - time[i];
    }
    durations[count - 1] = 0;  // Last note duration is 0 (or you can handle it differently)

    // Convert durations from seconds to milliseconds
    for (int i = 0; i < count; i++) {
        durations[i] *= 1000.0;
    }

    // Combine notes with very similar frequencies
    #define FREQUENCY_THRESHOLD 5.0  // Hz threshold for considering frequencies similar
    
    int new_count = 0;
    double combined_notes[MAX_LINES];
    double combined_durations[MAX_LINES];
    
    for (int i = 0; i < count; i++) {
        if (new_count == 0) {
            combined_notes[0] = notes[i];
            combined_durations[0] = durations[i];
            new_count = 1;
        } else {
            // Check if current note is similar to the last combined note
            if (fabs(notes[i] - combined_notes[new_count - 1]) < FREQUENCY_THRESHOLD) {
                // Combine durations
                combined_durations[new_count - 1] += durations[i];
            } else {
                // Add as new note
                combined_notes[new_count] = notes[i];
                combined_durations[new_count] = durations[i];
                new_count++;
            }
        }
    }
    
    // Copy combined notes back to original arrays
    for (int i = 0; i < new_count; i++) {
        notes[i] = combined_notes[i];
        durations[i] = combined_durations[i];
    }
    count = new_count;
    
    // Smooth out very short notes by combining them into the previous note
    #define MIN_NOTE_DURATION 50.0  // milliseconds - notes shorter than this will be merged
    
    new_count = 0;
    for (int i = 0; i < count; i++) {
        if (durations[i] < MIN_NOTE_DURATION && new_count > 0) {
            // Merge this short note into the previous note by extending its duration
            durations[new_count - 1] += durations[i];
        } else {
            // Keep this note as is
            if (new_count != i) {
                combined_notes[new_count] = notes[i];
                combined_durations[new_count] = durations[i];
            }
            new_count++;
        }
    }
    
    // Copy smoothed notes back to original arrays
    for (int i = 0; i < new_count; i++) {
        notes[i] = combined_notes[i];
        durations[i] = combined_durations[i];
    }
    count = new_count;

    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    while(!SharedData->BackStart&&!SharedData->KillMusic){
        usleep(250000);
    }

    SDL_PauseAudioDevice(dev, 0); // Start audio
    for(int j=0;j<1;j++){
        for (int n = 0; n < count; n++) {
            if(!SharedData->KillMusic){
                int samples = (int)((durations[n] / 1000.0) * SAMPLE_RATE);  // durations in ms, convert to seconds for sample calculation
                if (samples <= 0) continue;
                
                int16_t *buffer = malloc(samples * sizeof(int16_t));

                generate_square_wave(buffer, samples, notes[n]);
                SDL_QueueAudio(dev, buffer, samples * sizeof(int16_t));
                
                // Wait for this note to finish playing before freeing
                SDL_Delay((int)durations[n]);
                
                free(buffer);
            }

        }
    }
    //SDL_Delay(500); // Small pause at end
    SDL_CloseAudioDevice(dev);
    SDL_Quit();
    pthread_exit(NULL);
}
void generate_square_wave(int16_t *buffer, int samples, float frequency){
    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        buffer[i] = (sin(2 * M_PI * frequency * t) >= 0 ? AMPLITUDE : -AMPLITUDE);
    }
}
