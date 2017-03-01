/*
 * Copyright (c) 2016-present Shanghai Droi Technology Co., Ltd.
 * All rights reserved.
 */

#ifndef __DroiUser_H__
#define __DroiUser_H__
#include "DroiObject.h"

#define SIGNUP_USER_EXISTS 1040008
#define LOGIN_USER_NOT_EXISTS 1040009
#define LOGIN_USER_WRONG_PASSWORD 1040010
#define LOGIN_USER_DISABLE 1040011
#define DB_RECORD_CONFLIT 1030305


class DroiError;
class DroiUser : public DroiObject
{
public:
    /**
     *  Constructor a DroiUser object.
     * \return DroiUser instance
     */
    static DroiUser* create();

    /**
     *  Get cuurent login user.
     * \return DroiUser instance
     */
    static DroiUser* getCurrentUser();

    /**
     *  Login user with UserID and Password
     * \param userId UserId
     * \param password Password
     * \param error Pass DroiError to get error code. Pass null to ignore error details.
     * \return DroiUser instance.
     */
    static RefPtr<DroiUser> login( const std::string& userId, const std::string& password, DroiError* error );

    /**
     *  Login user with anonymous type.
     * \param error Pass DroiError to get error code. Pass null to ignore error details.
     */
    static RefPtr<DroiUser> loginWithAnonymous( DroiError* error );

    /**
     *  Logout current user. The logout is block action and need network connected.
     * \return DroiError object.
     */
    DroiError logout();

    /**
     *  Signup new user. The {@link #setUserId(String)} and {@link #setPassword(String)} must set before calling signup.
     * \return DroiError object.
     */
    DroiError signup();

    /**
     *  The user is anonymous.
     * \return true for anonlymous user.
     */
    bool isAnonymouseUser();

    /**
     *  The user login state.
     * \return true to login successfully and not expired.
     */
    bool isLoggedIn();

    /**
     *  To save data to cloud, the action must send with valid login user. Will auto login with anonymous user if not login before when saving data.
     * \param flag true to enable.
     */
    static void setAutoAnonymouseUser(bool flag);

    /**
     *  Get AutoAnonymousUserEnable state.
     * \return true to enable.
     */
    static bool isAutoAnonymousUserEnabled();

    /**
     *  Get isEnabled state.
     * \return true if it's enabled.
     */
    bool enabled();

    /**
     *  Set enable state
     * \param flag true to enable.
     */
    void setEnabled( bool enabled );

    /**
     *  Get user id.
     * \return User id.
     */
    std::string getUserId();

    /**
     *  Get user email.
     * \return User email.
     */
    std::string getEmail();

    /**
     *  Get user phone number.
     * \return User phone number.
     */
    std::string getPhoneNumber();

    /**
     *  Get user phone number.
     * \return User phone number.
     */
    std::string getSessionToken();

    /**
     *  Set user id. The id is for user signup.
     * \param userId User id
     */
    void setUserId( const std::string& userId );

    /**
     *  Set user password. The password is for user signup.
     * \param password User password.
     */
    void setPassword( const std::string& password );

    /**
     *  Set user email.
     * \param email User email.
     */
    void setEmail( const std::string& email );

    /**
     *  Set user phone number.
     * \param phoneNum User phone number.
     */
    void setPhoneNumber( const std::string& phoneNumber );

    /**
     *  Reset user password
     * \param oldPassword Old password to be check again
     * \param newPassword New password to be set
     * \return DroiError DroiError object. Developer should use isOk to check whether this result is OK.
     */
    DroiError changePassword( const std::string& oldPassword, const std::string newPassword);

    /**
     * Validate user email.
     *
     * \return DroiError DroiError object. Developer should use isOk to check whether this result is OK.
     */
    DroiError validateEmail();

    /**
     * Validate user phone number.
     *
     * \return DroiError DroiError object. Developer should use isOk to check whether this result is OK.
     */
    DroiError validatePhoneNumber();

    /**
     * Confirm pin code sent to user phone.
     *
     * \param pinCode Pic code
     * \return DroiError DroiError object. Developer should use isOk to check whether this result is OK.
     */
    DroiError confirmPhoneNumberPinCode( const std::string& pinCode );

    /**
     * Refersh EmailVerified / PhoneNumVerified field status. After refreshed, check verification via isEmailVerified or isPhoneNumVerified.
     *
     * \return true to successfully refreshed.
     */
    bool refreshValidationStatus();

    /**
     * Refersh EmailVerified / PhoneNumVerified field status in the background thread. After refreshed, check verification via isEmailVerified or isPhoneNumVerified.
     *
     * \param callback Callback
     * \return true to successfully push to background task to execute.
     */
    bool refreshValidationStatusInBackground(DroiCallback<RefPtr<DroiUser>>::onCallback2 callback);

    /**
     * Email field has been verified or not.
     *
     * \return true to EmailVerified is true or false otherwise.
     */
    bool isEmailVerified();

    /**
     * PhonNum field has been verified or not.
     *
     * \return true to PhoneNumVerified is true or false otherwise.
     */
    bool isPhoneNumVerified();
protected:
    DroiUser();

private:
    std::mutex csLocker;
    std::condition_variable csCondition;
    bool _EmailVerified;
    bool _PhoneNumVerified;

    friend class cJSONHelper;
    friend class CloudStorageDBHelper;
    friend class CloudUserStorageDBHelper;
    DroiError loginAnonymousInternal();
    void copyFromUser(DroiObject* user);
    
    static DroiUser* createAnonymouseUser();
    static void storeUser( DroiUser* user );
    static DroiUser* loadFromFile();
    static void cleanUp();
    static DroiUser* g_currentUser;
    ValueMap mSession;

    static bool mAutoAnonymousUserEnabled;
};

#endif
