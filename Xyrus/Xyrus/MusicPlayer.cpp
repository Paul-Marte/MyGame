//
// Created by David Burchill on 2022-11-29.
//

#include "MusicPlayer.h"
#include <stdexcept>


MusicPlayer::MusicPlayer() {
    _filenames["menuTheme"] = "../assets/Music/dp_progger.flac";  // TO CHANGE
    _filenames["gameTheme"] = "../assets/Music/game-music-loop-6-144641.ogg"; // TO CHANGE
}

void MusicPlayer::addSong(const std::string& name, const std::string& path) {
    _filenames[name] = path;
}

MusicPlayer& MusicPlayer::getInstance() {
    static MusicPlayer instance; // Meyers Singleton implementation
    return instance;
}


void MusicPlayer::play(String theme) {
    if (!_music.openFromFile(_filenames[theme]))
        throw std::runtime_error("Music could not open file");

    _music.setVolume(_volume);
    _music.setLoop(true);
    _music.play();
}


void MusicPlayer::stop() {
    _music.stop();
}


void MusicPlayer::setPaused(bool paused) {
    if (paused)
        _music.pause();
    else
        _music.play();
}


void MusicPlayer::setVolume(float volume) {
    _volume = volume;
    _music.setVolume(_volume);
}
