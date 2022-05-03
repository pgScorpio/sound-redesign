/******************************************************************************\
 * Copyright (c) 2004-2022
 *
 * Author(s):
 *  Volker Fischer
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
\******************************************************************************/

#pragma once

#include <QDomDocument>
#include <QFile>
#include <QSettings>
#include <QDir>
#include "global.h"
#include "cmdlnoptions.h"
#include "util.h"

/* Definitions ****************************************************************/
// audio in fader range
#define AUD_FADER_IN_MIN    0
#define AUD_FADER_IN_MAX    100
#define AUD_FADER_IN_MIDDLE ( AUD_FADER_IN_MAX / 2 )

// audio reverberation range
#define AUD_REVERB_MAX 100

/* Classes ********************************************************************/

class CSettings : public QObject
{
    Q_OBJECT

public:
    CSettings ( bool bIsClient, bool bUseGUI ) :
        CommandlineOptions(),
        vecWindowPosMain(), // empty array
        strLanguage(),
        strFileName()
    {
        if ( !CommandlineOptions.Load ( bIsClient, bUseGUI ) )
        {
#ifdef HEADLESS
            throw CErrorExit ( "Parameter Error(s), Exiting" );
#endif
        }
    }

    inline bool HaveGui() const { return !CommandlineOptions.nogui.IsSet(); }

public: // common settings
    CCommandlineOptions CommandlineOptions;

    QByteArray vecWindowPosMain;
    QString    strLanguage;

protected:
    QString strFileName;

protected:
    void SetFileName ( const QString& sNFiName, const QString& sDefaultFileName );

    void Load();
    void Save();

    void ReadFromFile ( const QString& strCurFileName, QDomDocument& XMLDocument );
    void WriteToFile ( const QString& strCurFileName, const QDomDocument& XMLDocument );

    // The following functions implement the conversion from the general string
    // to base64 (which should be used for binary data in XML files). This
    // enables arbitrary utf8 characters to be used as the names in the GUI.
    //
    // ATTENTION: The "FromBase64[...]" functions must be used with caution!
    //            The reason is that if the FromBase64ToByteArray() is used to
    //            assign the stored value to a QString, this is incorrect but
    //            will not generate a compile error since there is a default
    //            conversion available for QByteArray to QString.
    QString    ToBase64 ( const QByteArray strIn ) const { return QString::fromLatin1 ( strIn.toBase64() ); }
    QString    ToBase64 ( const QString strIn ) const { return ToBase64 ( strIn.toUtf8() ); }
    QByteArray FromBase64ToByteArray ( const QString strIn ) const { return QByteArray::fromBase64 ( strIn.toLatin1() ); }
    QString    FromBase64ToString ( const QString strIn ) const { return QString::fromUtf8 ( FromBase64ToByteArray ( strIn ) ); }

    // init file access function for read/write
    void SetNumericIniSet ( QDomDocument& xmlFile, const QString& strSection, const QString& strKey, const int iValue = 0 );

    bool GetNumericIniSet ( const QDomDocument& xmlFile,
                            const QString&      strSection,
                            const QString&      strKey,
                            const int           iRangeStart,
                            const int           iRangeStop,
                            int&                iValue );

    bool GetFlagIniSet ( const QDomDocument& xmlFile, const QString& strSection, const QString& strKey, bool& bValue );

    void SetFlagIniSet ( QDomDocument& xmlFile, const QString& strSection, const QString& strKey, const bool bValue = false );

    // actual working function for init-file access
    QString GetIniSetting ( const QDomDocument& xmlFile, const QString& sSection, const QString& sKey, const QString& sDefaultVal = "" );

    void PutIniSetting ( QDomDocument& xmlFile, const QString& sSection, const QString& sKey, const QString& sValue = "" );

protected:
    virtual void WriteSettingsToXML ( QDomDocument& IniXMLDocument )        = 0;
    virtual void ReadSettingsFromXML ( const QDomDocument& IniXMLDocument ) = 0;
};

#ifndef SERVER_ONLY
class CAudioDeviceSettings
{
public:
    CAudioDeviceSettings() :
        strName ( "" ),
        iLeftInputChannel ( 0 ),
        iRightInputChannel ( 1 ),
        iLeftOutputChannel ( 0 ),
        iRightOutputChannel ( 1 ),
        iPrefFrameSizeFactor ( 128 ),
        iInputBoost ( 1 )
    {}

public:
    QString strName;
    int     iLeftInputChannel;
    int     iRightInputChannel;
    int     iLeftOutputChannel;
    int     iRightOutputChannel;
    int     iPrefFrameSizeFactor;
    int     iInputBoost;
    // int  iLeftInputBoost;
    // int  iRightInputBoost;
};

class CClientSettings : public CSettings
{
    Q_OBJECT

public:
    CClientSettings ( bool bUseGUI ) :
        CSettings ( true, bUseGUI ),
        cAudioDevice(),
        vecStoredFaderTags ( MAX_NUM_STORED_FADER_SETTINGS, "" ),
        vecStoredFaderLevels ( MAX_NUM_STORED_FADER_SETTINGS, AUD_MIX_FADER_MAX ),
        vecStoredPanValues ( MAX_NUM_STORED_FADER_SETTINGS, AUD_MIX_PAN_MAX / 2 ),
        vecStoredFaderIsSolo ( MAX_NUM_STORED_FADER_SETTINGS, false ),
        vecStoredFaderIsMute ( MAX_NUM_STORED_FADER_SETTINGS, false ),
        vecStoredFaderGroupID ( MAX_NUM_STORED_FADER_SETTINGS, INVALID_INDEX ),
        vstrIPAddress ( MAX_NUM_SERVER_ADDR_ITEMS, "" ),
        iNewClientFaderLevel ( 100 ),
        iSettingsTab ( SETTING_TAB_AUDIONET ),
        bConnectDlgShowAllMusicians ( true ),
        eChannelSortType ( ST_NO_SORT ),
        iNumMixerPanelRows ( 1 ),
        vstrDirectoryAddress ( MAX_NUM_SERVER_ADDR_ITEMS, "" ),
        eDirectoryType ( AT_DEFAULT ),
        bEnableFeedbackDetection ( true ),
        vecWindowPosSettings(), // empty array
        vecWindowPosChat(),     // empty array
        vecWindowPosConnect(),  // empty array
        bWindowWasShownSettings ( false ),
        bWindowWasShownChat ( false ),
        bWindowWasShownConnect ( false ),
        bOwnFaderFirst ( false ),
        ChannelInfo(),
        eAudioQuality ( AQ_NORMAL ),
        eAudioChannelConfig ( CC_MONO ),
        eGUIDesign ( GD_ORIGINAL ),
        eMeterStyle ( MT_LED_STRIPE ),
        bEnableOPUS64 ( false ),
        iAudioInputBalance ( AUD_FADER_IN_MIDDLE ),
        bReverbOnLeftChan ( false ),
        iReverbLevel ( 0 ),
        iServerSockBufNumFrames ( DEF_NET_BUF_SIZE_NUM_BL ),
        bFraSiFactPrefSupported ( false ),
        bFraSiFactDefSupported ( false ),
        bFraSiFactSafeSupported ( false ),
        bMuteOutStream ( false ),
        // Status values
        strServerAddress(),
        strServerName(),
        bConnectRequested ( false ),
        bDisconnectRequested ( false ),
        bConnectionEnabled ( false ),
        bConnected ( false )
    {
        SetFileName ( CommandlineOptions.inifile.Value(), DEFAULT_INI_FILE_NAME );
        Load();
        // SelectSoundCard ( strCurrentAudioDevice );
    }

    ~CClientSettings() { Save(); }

public:                        // Values without notifiers: (these don't need direct action on change, they are used 'on the fly')
    int iCustomDirectoryIndex; // index of selected custom directory server

    int  iNewClientFaderLevel;
    bool bConnectDlgShowAllMusicians;

    CVector<QString> vecStoredFaderTags;
    CVector<int>     vecStoredFaderLevels;
    CVector<int>     vecStoredPanValues;
    CVector<int>     vecStoredFaderIsSolo;
    CVector<int>     vecStoredFaderIsMute;
    CVector<int>     vecStoredFaderGroupID;
    CVector<QString> vstrIPAddress;

    EChSortType    eChannelSortType;
    EDirectoryType eDirectoryType;

    bool bEnableFeedbackDetection;

public: // window position/state settings
    QByteArray vecWindowPosSettings;
    QByteArray vecWindowPosChat;
    QByteArray vecWindowPosConnect;
    bool       bWindowWasShownSettings;
    bool       bWindowWasShownChat;
    bool       bWindowWasShownConnect;
    int        iSettingsTab;

public:
    // CustomDirectories
    // Special case:
    //     there are many ways vstrDirectoryAddress can be changed,
    //     so, after changing this one, one should always call OnCustomDirectoriesChanged() !!
    CVector<QString> vstrDirectoryAddress;
    void             OnCustomDirectoriesChanged() { emit CustomDirectoriesChanged(); }

protected: // values with notifiers: use Get/Set functions !
    //### TODO: BEGIN ###//
    // After the sound-redesign we should store all Soundcard settings per device (and per device type!),
    // i.e. in a CVector< CSoundCardSettings > SoundCard[],
    // and set a current Device: CSoundCardSettings CurrentSoundCard = SoundCard[n]
    // CurrentSoundCard.strName will be stored in the inifile as 'CurrentSoundCard'
    // and there should be a function Settings.SelectSoundCard ( strName )
    // there should also be a function Settings.StoreSoundCard( CSoundCardSettings& sndCardSettings)
    // Which will store sndCardSettings in the matching SoundCard[x], or add a new device to
    // SoundCard[] when no SoundCard[x] with sndCardSettings.strName already exists.
    //
    CAudioDeviceSettings cAudioDevice;
    //### TODO: END ###//

    EGUIDesign  eGUIDesign;
    EMeterStyle eMeterStyle;

    EAudChanConf  eAudioChannelConfig;
    EAudioQuality eAudioQuality;

    CChannelCoreInfo ChannelInfo;

    int  iClientSockBufNumFrames;
    int  iServerSockBufNumFrames;
    bool bAutoSockBufSize;

    bool bEnableOPUS64;

    int iNumMixerPanelRows;

    int  iAudioInputBalance;
    int  iReverbLevel;
    bool bReverbOnLeftChan;

    bool bOwnFaderFirst;

signals:
    // Settings changed signals

    void CustomDirectoriesChanged();

    void InputBoostChanged();

    void AudioDeviceChanged();
    void InputChannelChanged();
    void OutputChannelChanged();
    void PrefFrameSizeFactorChanged();

    void GUIDesignChanged();
    void MeterStyleChanged();

    void AudioChannelConfigChanged();
    void AudioQualityChanged();

    void ChannelInfoChanged();

    void EnableOPUS64Changed();

    void ClientSockBufNumFramesChanged();
    void ServerSockBufNumFramesChanged();
    void AutoSockBufSizeChanged();

    void NumMixerPanelRowsChanged();

    void AudioInputBalanceChanged();

    void ReverbLevelChanged();
    void ReverbChannelChanged();

    void OwnFaderFirstChanged();

    void OpenDriverSetup(); // Just needed for signalling, no related value

    // State signals
    void Connecting();
    void Disconnecting();

    void Connected();
    void Disconnected();

    // Request signals to CClient
    void ConnectRequested();
    void DisconnectRequested();

public:
    inline const QString& GetClientName() const { return CommandlineOptions.clientname.Value(); }

    inline const QString GetWindowTitle()
    {
        if ( GetClientName().isEmpty() )
        {
            return QString ( APP_NAME );
        }
        else
        {
            return QString ( APP_NAME ) + " - " + GetClientName();
        }
    }

    inline QString GetAudioDevice() const { return cAudioDevice.strName; }
    bool           SetAudioDevice ( QString deviceName, bool bReinit = false )
    {
        if ( bReinit || ( cAudioDevice.strName != deviceName ) )
        {
            //### TODO: BEGIN ###//
            // get complete cAudioDevice for this device!
            cAudioDevice.strName = deviceName;
            //### TODO: END ###//
            emit AudioDeviceChanged();

            return true;
        }

        return false;
    }

    inline int GetInputBoost( /* bool bRight */ ) const { return cAudioDevice.iInputBoost; }

    bool SetInputBoost ( /* bool bRight */ int boost )
    {
        if ( cAudioDevice.iInputBoost != boost )
        {
            cAudioDevice.iInputBoost = boost;
            emit InputBoostChanged();

            return true;
        }

        return false;
    }

    int GetInputChannel ( bool bRight ) const
    {
        if ( bRight )
        {
            return cAudioDevice.iRightInputChannel;
        }
        else
        {
            return cAudioDevice.iLeftInputChannel;
        }
    }

    bool SetInputChannel ( bool bRight, int chNum )
    {
        if ( bRight )
        {
            if ( cAudioDevice.iRightInputChannel != chNum )
            {
                cAudioDevice.iRightInputChannel = chNum;
                emit InputChannelChanged();

                return true;
            }
        }
        else
        {
            if ( cAudioDevice.iLeftInputChannel != chNum )
            {
                cAudioDevice.iLeftInputChannel = chNum;
                emit InputChannelChanged();

                return true;
            }
        }

        return false;
    }

    int GetOutputChannel ( bool bRight ) const
    {
        if ( bRight )
        {
            return cAudioDevice.iRightOutputChannel;
        }
        else
        {
            return cAudioDevice.iLeftOutputChannel;
        }
    }

    bool SetOutputChannel ( bool bRight, int chNum )
    {
        if ( bRight )
        {
            if ( cAudioDevice.iRightOutputChannel != chNum )
            {
                cAudioDevice.iRightOutputChannel = chNum;
                emit OutputChannelChanged();

                return true;
            }
        }
        else
        {
            if ( cAudioDevice.iLeftOutputChannel != chNum )
            {
                cAudioDevice.iLeftOutputChannel = chNum;
                emit OutputChannelChanged();

                return true;
            }
        }

        return false;
    }

    inline int GetSndCrdPrefFrameSizeFactor() const { return cAudioDevice.iPrefFrameSizeFactor; }
    bool       SetSndCrdPrefFrameSizeFactor ( int iSize )
    {
        if ( cAudioDevice.iPrefFrameSizeFactor != iSize )
        {
            cAudioDevice.iPrefFrameSizeFactor = iSize;
            emit PrefFrameSizeFactorChanged();

            return true;
        }

        return false;
    }

    inline EGUIDesign GetGUIDesign() const { return eGUIDesign; }
    bool              SetGUIDesign ( EGUIDesign design )
    {
        if ( eGUIDesign != design )
        {
            eGUIDesign = design;
            emit GUIDesignChanged();

            return true;
        }

        return false;
    }

    inline EMeterStyle GetMeterStyle() const { return eMeterStyle; }
    bool               SetMeterStyle ( EMeterStyle style )
    {
        if ( eMeterStyle != style )
        {
            eMeterStyle = style;
            emit MeterStyleChanged();

            return true;
        }

        return false;
    }

    inline EAudChanConf GetAudioChannelConfig() const { return eAudioChannelConfig; }
    bool                SetAudioChannelConfig ( EAudChanConf config )
    {
        if ( eAudioChannelConfig != config )
        {
            eAudioChannelConfig = config;
            emit AudioChannelConfigChanged();

            return true;
        }

        return false;
    }

    inline EAudioQuality GetAudioQuality() const { return eAudioQuality; }
    bool                 SetAudioQuality ( EAudioQuality quality )
    {
        if ( eAudioQuality != quality )
        {
            eAudioQuality = quality;
            emit AudioQualityChanged();

            return true;
        }

        return false;
    }

    inline CChannelCoreInfo& GetChannelInfo() { return ChannelInfo; };
    bool                     SetChannelInfo ( const CChannelCoreInfo& info )
    {
        ChannelInfo = info;
        emit ChannelInfoChanged();

        return true;
    }

    inline const QString& GetChannelInfoName() const { return ChannelInfo.strName; }
    bool                  SetChannelInfoName ( const QString& name )
    {
        if ( ChannelInfo.strName != name )
        {
            ChannelInfo.strName = name;
            emit ChannelInfoChanged();

            return true;
        }

        return false;
    }

    inline const QLocale::Country GetChannelInfoCountry() const { return ChannelInfo.eCountry; }
    bool                          SetChannelInfoCountry ( const QLocale::Country country )
    {
        if ( ChannelInfo.eCountry != country )
        {
            ChannelInfo.eCountry = country;
            emit ChannelInfoChanged();

            return true;
        }

        return false;
    }

    inline const QString& GetChannelInfoCity() const { return ChannelInfo.strCity; }
    bool                  SetChannelInfoCity ( const QString& city )
    {
        if ( ChannelInfo.strCity != city )
        {
            ChannelInfo.strCity = city;
            emit ChannelInfoChanged();

            return true;
        }

        return false;
    }

    inline int GetChannelInfoInstrument() const { return ChannelInfo.iInstrument; }
    bool       SetChannelInfoInstrument ( const int instrument )
    {
        if ( ChannelInfo.iInstrument != instrument )
        {
            ChannelInfo.iInstrument = instrument;
            emit ChannelInfoChanged();

            return true;
        }

        return false;
    }

    inline ESkillLevel GetChannelInfoSkillLevel() const { return ChannelInfo.eSkillLevel; }
    bool               SetChannelInfoSkillLevel ( ESkillLevel skillLevel )
    {
        if ( ChannelInfo.eSkillLevel != skillLevel )
        {
            ChannelInfo.eSkillLevel = skillLevel;
            emit ChannelInfoChanged();

            return true;
        }

        return false;
    }

    inline int GetClientSockBufNumFrames() const { return iClientSockBufNumFrames; }
    bool       SetClientSockBufNumFrames ( int numFrames )
    {
        if ( iClientSockBufNumFrames != numFrames )
        {
            iClientSockBufNumFrames = numFrames;
            emit ClientSockBufNumFramesChanged();

            return true;
        }

        return false;
    }

    inline int GetServerSockBufNumFrames() const { return iServerSockBufNumFrames; }
    bool       SetServerSockBufNumFrames ( int numFrames )
    {
        if ( iServerSockBufNumFrames != numFrames )
        {
            iServerSockBufNumFrames = numFrames;
            emit ServerSockBufNumFramesChanged();

            return true;
        }

        return false;
    }

    inline bool GetAutoSockBufSize() const { return bAutoSockBufSize; }
    bool        SetAutoSockBufSize ( bool bAuto )
    {
        if ( bAutoSockBufSize != bAuto )
        {
            bAutoSockBufSize = bAuto;
            emit AutoSockBufSizeChanged();

            return true;
        }

        return false;
    }

    inline bool GetEnableOPUS64() const { return bEnableOPUS64; }
    bool        SetEnableOPUS64 ( bool bEnable )
    {
        if ( bEnableOPUS64 != bEnable )
        {
            bEnableOPUS64 = bEnable;
            emit EnableOPUS64Changed();

            return true;
        }

        return false;
    }

    inline int GetNumMixerPanelRows() const { return iNumMixerPanelRows; }
    bool       SetNumMixerPanelRows ( int rows )
    {
        if ( iNumMixerPanelRows != rows )
        {
            iNumMixerPanelRows = rows;
            emit NumMixerPanelRowsChanged();

            return true;
        }

        return false;
    }

    inline int GetAudioInputBalance() const { return iAudioInputBalance; }
    bool       SetAudioInputBalance ( int iValue )
    {
        if ( iAudioInputBalance != iValue )
        {
            iAudioInputBalance = iValue;
            emit AudioInputBalanceChanged();

            return true;
        }

        return false;
    }

    inline int GetReverbLevel() const { return iReverbLevel; }
    bool       SetReverbLevel ( int iLevel )
    {
        if ( iReverbLevel != iLevel )
        {
            iReverbLevel = iLevel;
            emit ReverbLevelChanged();

            return true;
        }

        return false;
    }

    inline bool GetReverbOnLeftChannel() const { return bReverbOnLeftChan; }
    bool        SetReverbOnLeftChannel ( bool bOnLeftChannel )
    {
        if ( bReverbOnLeftChan != bOnLeftChannel )
        {
            bReverbOnLeftChan = bOnLeftChannel;
            emit ReverbChannelChanged();

            return true;
        }

        return false;
    }

    inline bool GetOwnFaderFirst() const { return bOwnFaderFirst; }
    bool        SetOwnFaderFirst ( bool bOwnFirst )
    {
        if ( bOwnFaderFirst != bOwnFirst )
        {
            bOwnFaderFirst = bOwnFirst;
            emit OwnFaderFirstChanged();

            return true;
        }

        return false;
    }

    /*
    protected:
        void SelectSoundCard ( QString strName );
        void StoreSoundCard ( CSoundCardSettings& sndCardSettings );
    */

public: // Unsaved settings, needed by CClientSettingsDlg
    //### TODO: BEGIN ###//
    // these could be set by CSound on device selection or included in SoundProperties !
    // no more need for separate calls to CSound from CClient.Init() then.
    bool bFraSiFactPrefSupported;
    bool bFraSiFactDefSupported;
    bool bFraSiFactSafeSupported;
    //### TODO: END ###//

    bool bMuteOutStream;

public:
    void LoadFaderSettings ( const QString& strCurFileName );
    void SaveFaderSettings ( const QString& strCurFileName );

    void RequestDriverSetup() { emit OpenDriverSetup(); }

    // void SelectSoundCard ( QString strName );
    // void StoreSoundCard ( CSoundCardSettings& sndCardSettings );

protected:
    // Status values
    QString strServerAddress;
    QString strServerName;

    bool bConnectRequested;
    bool bDisconnectRequested;
    bool bConnectionEnabled; // true if we are Connecting or Connected, false if we are Disconnecting or Disconnected
    bool bConnected;

public:
    // Status values

    //### TODO: BEGIN ###//
    // Use a separate Status class for Client and Server
    // These classes should provide control and status values like connection status, ping time, overall delay, etc...

    inline const QString GetServerAddress() const { return strServerAddress; }
    inline const QString GetServerName() const { return strServerName; }
    inline bool          GetConnectionEnabled() const { return bConnectionEnabled || bConnectRequested; }
    bool                 StartConnection ( const QString& serverAddress, const QString& serverName )
    {
        if ( !bConnectionEnabled && !bConnectRequested && !serverAddress.isEmpty() )
        {
            strServerAddress  = serverAddress;
            strServerName     = serverName.isEmpty() ? serverAddress : serverName;
            bConnectRequested = true;
            emit ConnectRequested();

            return true;
        }

        return false;
    }

    bool EndConnection()
    {
        if ( bConnectionEnabled )
        {
            if ( !bDisconnectRequested )
            {
                bDisconnectRequested = true;
                emit DisconnectRequested();
            }

            return true;
        }

        return false;
    }

    void AckConnecting ( bool ack )
    {
        if ( bConnectRequested )
        {
            bConnectRequested  = false;
            bConnectionEnabled = ack;

            if ( ack )
            {
                emit Connecting();
            }
            else
            {
                bConnected = false;
            }
        }
    }

    void AckDisconnecting ( bool ack )
    {
        if ( bDisconnectRequested )
        {
            bDisconnectRequested = false;

            if ( ack )
            {
                emit Disconnecting();
            }
        }
    }

    inline bool GetConnected() const { return bConnected; }
    void        SetConnected ( bool bState = true )
    {
        bState &= bConnectionEnabled; // can't be connected if connection is not enabled !

        if ( bConnected != bState )
        {
            bConnected = bState;
            if ( bConnected )
            {
                emit Connected();
            }
            else
            {
                bConnectionEnabled = false;
                emit Disconnected();
            }
        }
    }

    //### TODO: END ###//

protected:
    void ReadFaderSettingsFromXML ( const QDomDocument& IniXMLDocument );
    void WriteFaderSettingsToXML ( QDomDocument& IniXMLDocument );

protected:
    virtual void ReadSettingsFromXML ( const QDomDocument& IniXMLDocument ) override;
    virtual void WriteSettingsToXML ( QDomDocument& IniXMLDocument ) override;
};
#endif

class CServerSettings : public CSettings
{
public:
    CServerSettings ( bool bUseGUI ) :
        CSettings ( false, bUseGUI ),
        strServerName ( "" ),
        strServerCity ( "" ),
        eServerCountry ( QLocale::Country::UnitedStates ),
        bEnableRecording ( false ),
        strWelcomeMessage ( "" ),
        strRecordingDir(),
        strDirectoryAddress(),
        eDirectoryType ( AT_NONE ),
        strServerListFileName(),
        bAutoRunMinimized ( false ),
        bDelayPan ( false )
    {
        SetFileName ( CommandlineOptions.inifile.Value(), DEFAULT_INI_FILE_NAME_SERVER );
        Load();
    }

    ~CServerSettings() { Save(); }

protected:
    QString          strServerName;
    QString          strServerCity;
    QLocale::Country eServerCountry;
    bool             bEnableRecording;
    QString          strWelcomeMessage;
    QString          strRecordingDir;
    QString          strDirectoryAddress;
    EDirectoryType   eDirectoryType;
    QString          strServerListFileName;
    bool             bAutoRunMinimized;
    bool             bDelayPan;

public:
    const QString    GetServerName() const { return strServerName; }
    const QString    GetServerCity() const { return strServerCity; }
    QLocale::Country GetServerCountry() const { return eServerCountry; }

    bool GetEnableRecording() const { return CommandlineOptions.norecord.IsSet() ? false : bEnableRecording; }
    void SetEnableRecording ( bool newEnableRecording )
    {
        CommandlineOptions.norecord.Unset();

        if ( bEnableRecording != newEnableRecording )
        {
            bEnableRecording = newEnableRecording;
            // todo: emit Changed
        }
    }

    const QString GetWelcomeMessage() const
    {
        return CommandlineOptions.welcomemessage.IsSet() ? CommandlineOptions.welcomemessage.Value() : strWelcomeMessage;
    }
    void SetWelcomeMessage ( const QString& newWelcomeMessage )
    {
        CommandlineOptions.welcomemessage.Unset();

        if ( strWelcomeMessage != newWelcomeMessage )
        {
            strWelcomeMessage = newWelcomeMessage;
            // todo: emit Changed
        }
    }

    const QString GetRecordingDir() const { return CommandlineOptions.recording.IsSet() ? CommandlineOptions.recording.Value() : strRecordingDir; }
    void          SetRecordingDir ( const QString& newRecordingDir )
    {
        CommandlineOptions.recording.Unset();

        if ( strRecordingDir != newRecordingDir )
        {
            strRecordingDir != newRecordingDir;
            // todo: emit Changed
        }
    }

    const QString GetDirectoryAddress() const
    {
        return CommandlineOptions.directoryserver.IsSet() ? CommandlineOptions.directoryserver.Value() : strDirectoryAddress;
    }

    void SetDirectoryAddress ( const QString& strNewAddress )
    {
        CommandlineOptions.directoryserver.Unset();

        if ( strDirectoryAddress != strNewAddress )
        {
            strDirectoryAddress = strNewAddress;
            // todo: emit Changed
        }
    }

    EDirectoryType GetDirectoryType() const { return eDirectoryType; }
    void           SetDirectoryType ( EDirectoryType newDirectoryType )
    {
        if ( eDirectoryType != newDirectoryType )
        {
            eDirectoryType = newDirectoryType;
            // todo: emit Changed
        }
    }

    const QString GetServerListFileName() const
    {
        return CommandlineOptions.directoryfile.IsSet() ? CommandlineOptions.directoryfile.Value() : strServerListFileName;
    }
    void SetServerListFileName ( const QString& strNewServerListFileName )
    {
        CommandlineOptions.directoryfile.Unset();

        if ( strServerListFileName != strNewServerListFileName )
        {
            strServerListFileName = strNewServerListFileName;
            // todo: emit Changed
        }
    }
    bool GetAutoRunMinimized() const { return CommandlineOptions.startminimized.IsSet() ? true : bAutoRunMinimized; }
    void SetAutoRunMinimized ( bool newAutoRunMinimized )
    {
        CommandlineOptions.startminimized.Unset();

        if ( bAutoRunMinimized != newAutoRunMinimized )
        {
            bAutoRunMinimized = newAutoRunMinimized;
            // todo: emit Changed
        }
    }

    bool GetDelayPan() const { return CommandlineOptions.delaypan.IsSet() ? true : bDelayPan; }
    void SetDelayPan ( bool newDelayPan )
    {
        CommandlineOptions.delaypan.Unset();

        if ( bDelayPan != newDelayPan )
        {
            bDelayPan = newDelayPan;
            // todo: emit Changed
        }
    }
    //### TODO: END ###//

public:
    inline const QString GetWindowTitle()
    {
        if ( GetServerName().isEmpty() )
        {
            return QString ( APP_NAME ) + "Server";
        }
        else
        {
            return QString ( APP_NAME ) + "Server - " + GetServerName();
        }
    }

protected:
    virtual void ReadSettingsFromXML ( const QDomDocument& IniXMLDocument ) override;
    virtual void WriteSettingsToXML ( QDomDocument& IniXMLDocument ) override;
};
