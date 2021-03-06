/*
 * OpenAL sound file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenAL/spOpenALSound.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "Base/spMemoryManagement.hpp"
#include "Base/spInputOutputLog.hpp"
#include "SoundSystem/OpenAL/spOpenALSoundDevice.hpp"
#include "SoundSystem/OpenAL/spOpenALBufferObject.hpp"
#include "SoundSystem/OpenAL/spOpenALExtensions.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern audio::SoundDevice* GlbSoundSys;

namespace audio
{


OpenALSound::OpenALSound() :
    Sound           (   ),
    BufferObject_   (0  )
{
}
OpenALSound::~OpenALSound()
{
    close();
}

bool OpenALSound::reload(const io::stringc &Filename, u8 BufferCount)
{
    if (!checkFile(Filename))
        return false;
    
    /* Close previous loaded sound */
    close();
    
    /* Create new OpenAL buffer object */
    BufferObject_ = static_cast<OpenALSoundDevice*>(GlbSoundSys)->createSoundBuffer(Filename);
    
    if (!BufferObject_)
    {
        io::Log::lowerTab();
        return false;
    }
    
    /* Create new OpenAL source objects */
    SourceObjects_  .resize(BufferCount);
    SourceObjectIDs_.resize(BufferCount);
    
    for (u8 i = 0; i < BufferCount; ++i)
    {
        SourceObjects_[i] = MemoryManager::createMemory<ALSourceObject>("OpenALSound::ALSourceObject");
        SourceObjects_[i]->setupBuffer(BufferObject_);
        SourceObjectIDs_[i] = SourceObjects_[i]->getID();
    }
    
    /* Initialize source object iterator */
    CurSourceObject_ = SourceObjects_.begin();
    
    Filename_ = Filename;
    io::Log::lowerTab();
    
    return true;
}

void OpenALSound::close()
{
    static_cast<OpenALSoundDevice*>(GlbSoundSys)->dropSoundBuffer(BufferObject_);
    
    MemoryManager::deleteList(SourceObjects_);
    SourceObjectIDs_.clear();
}

void OpenALSound::play()
{
    Sound::play();
    
    if (!SourceObjects_.empty())
    {
        const ALuint ALSource = nextSourceBuffer();
        
        alSourcef(ALSource, AL_PITCH, GlbSoundSys->getListenerSpeed());
        alSourcef(ALSource, AL_ROLLOFF_FACTOR, 0.5f);//!!!
        
        alSourcePlay(ALSource);
    }
}

void OpenALSound::pause(bool Paused)
{
    Sound::pause(Paused);
    
    if (!SourceObjectIDs_.empty())
    {
        if (Paused)
            alSourcePausev(SourceObjectIDs_.size(), &SourceObjectIDs_[0]);
        else
            alSourcePlayv(SourceObjectIDs_.size(), &SourceObjectIDs_[0]);
    }
}

void OpenALSound::stop()
{
    Sound::stop();
    
    if (!SourceObjectIDs_.empty())
        alSourceRewindv(SourceObjectIDs_.size(), &SourceObjectIDs_[0]);
}

void OpenALSound::emit2D(f32 Volume, bool UseEffectSlot)
{
    const ALuint ALSource = nextSourceBuffer();
    
    if (UseEffectSlot)
    {
        const ALuint ALEffectSlot = static_cast<OpenALSoundDevice*>(GlbSoundSys)->ALEffectSlot_;
        alSource3i(ALSource, AL_AUXILIARY_SEND_FILTER, ALEffectSlot, 0, AL_FILTER_NULL);
    }
    else
        alSource3i(ALSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
    
    const f32 Point[3] = { 0.0f, 0.0f, 0.0f };
    
    alSourcef(ALSource, AL_PITCH, GlbSoundSys->getListenerSpeed());
    alSourcei(ALSource, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcef(ALSource, AL_GAIN, Volume);
    alSourcefv(ALSource, AL_POSITION, Point);
    alSourcef(ALSource, AL_ROLLOFF_FACTOR, 0.5f);//!!!
    
    alSourcePlay(ALSource);
}

void OpenALSound::emit3D(const dim::vector3df &Point, f32 Volume, bool UseEffectSlot)
{
    const ALuint ALSource = nextSourceBuffer();
    
    if (UseEffectSlot)
    {
        const ALuint ALEffectSlot = static_cast<OpenALSoundDevice*>(GlbSoundSys)->ALEffectSlot_;
        alSource3i(ALSource, AL_AUXILIARY_SEND_FILTER, ALEffectSlot, 0, AL_FILTER_NULL);
    }
    else
        alSource3i(ALSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
    
    alSourcef(ALSource, AL_PITCH, GlbSoundSys->getListenerSpeed());
    alSourcei(ALSource, AL_SOURCE_RELATIVE, AL_FALSE);
    alSourcef(ALSource, AL_GAIN, Volume);
    alSourcefv(ALSource, AL_POSITION, &Point.X);
    alSourcef(ALSource, AL_ROLLOFF_FACTOR, 0.5f);//!!!
    
    alSourcePlay(ALSource);
}

void OpenALSound::setSeek(f32 Seek)
{
    if (!SourceObjects_.empty())
        alSourcef(getSourceID(), AL_SEC_OFFSET, Seek * getLength());
}

f32 OpenALSound::getSeek() const
{
    if (!SourceObjects_.empty())
    {
        f32 Seek = 0.0f;
        alGetSourcef(getSourceID(), AL_SEC_OFFSET, &Seek);
        return Seek / getLength();
    }
    
    return 0.0f;
}

void OpenALSound::setVolume(f32 Volume)
{
    Sound::setVolume(Volume);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcef(ALSource, AL_GAIN, Volume);
}

void OpenALSound::setSpeed(f32 Speed)
{
    Sound::setSpeed(Speed);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcef(ALSource, AL_PITCH, Speed);
}

void OpenALSound::setBalance(f32 Balance)
{
    //todo
}

void OpenALSound::setLoop(bool Enable)
{
    Sound::setLoop(Enable);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcei(ALSource, AL_LOOPING, isLoop_);
}

f32 OpenALSound::getLength() const
{
    return 1.0f; //todo
}

bool OpenALSound::valid() const
{
    return BufferObject_ && alIsBuffer(BufferObject_->getID());
}

void OpenALSound::setPosition(const dim::vector3df &Position)
{
    Sound::setPosition(Position);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcefv(ALSource, AL_POSITION, &Position.X);
}
void OpenALSound::setVelocity(const dim::vector3df &Veclotiy)
{
    Sound::setVelocity(Veclotiy);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcefv(ALSource, AL_VELOCITY, &Veclotiy.X);
}

void OpenALSound::setVolumetric(bool isVolumetric)
{
    Sound::setVolumetric(isVolumetric);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcei(ALSource, AL_SOURCE_RELATIVE, isVolumetric ? AL_FALSE : AL_TRUE);
}
void OpenALSound::setVolumetricRadius(f32 Radius)
{
    Sound::setVolumetricRadius(Radius);
    
    foreach (ALuint ALSource, SourceObjectIDs_)
        alSourcef(ALSource, AL_ROLLOFF_FACTOR, 100.0f / Radius);
}

bool OpenALSound::playing() const
{
    return !SourceObjects_.empty() && (*CurSourceObject_)->getState() == AL_PLAYING;
}
bool OpenALSound::finish() const
{
    return !playing(); //todo
}


/*
 * ======= Private: =======
 */

ALuint OpenALSound::nextSourceBuffer()
{
    if (SourceObjects_.size() > 1)
    {
        ++CurSourceObject_;
        if (CurSourceObject_ == SourceObjects_.end())
            CurSourceObject_ = SourceObjects_.begin();
    }
    return getSourceID();
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
