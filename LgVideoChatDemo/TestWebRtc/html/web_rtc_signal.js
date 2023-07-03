var gstrUserId = "";
var gstrUserPasswd = "";
var gstrUserPasswd1 = "";
var gstrUserPasswd2 = "";
var gstrOtpNum = "";
var gstrToId = "";
var gstrSdp = "";
var ws = null;

btnRetreiveList.disabled = true;
btnChangeEmail.disabled = true;
btnChangePassword.disabled = true;
btnInvite.disabled = true;
btnAccept.disabled = true;
btnDecline.disabled = true;
btnBye.disabled = true;

function CheckDuplicateId() {
    let websocket = null;
    let UserId = document.getElementById('userid_id');

    if (UserId.value.length == 0) {
        Log("User ID has not been entered");
        alert("User ID has not been entered");
        return;
    }
    if (UserId.value.includes('|')) {
        Log("userid can NOT include '|' character");
        alert("userid can NOT include '|' character");
        return;
    }

    if (websocket == null) {
        if (window.location.protocol == "https:") {
            websocket = new WebSocket("wss://" + window.location.hostname);
        }
        else {
            return;
        }

        websocket.onopen = function (e) {
            websocket.send("req|check|" + UserId.value)
        };
        websocket.onmessage = function (e) {
            Log("Recv[" + e.data + "]");

            var arrData = e.data.split("|");

            websocket.close();
            switch (arrData[0]) {
                case "res":
                    switch (arrData[1]) {
                        case "check":
                            if (arrData[2] == '200') {
                                alert("user ID is available.[" + UserId.value + "]");
                            }
                            else {
                                var iStatusCode = parseInt(arrData[2]);

                                if (iStatusCode == 400) {
                                    Log("user ID is ALREADY registered");
                                    alert("user ID is ALREADY registered\r\nPlease use another user ID");
                                }
                                else if (iStatusCode == 410) {
                                    alert("userid has not been entered");
                                }
                            }
                            break;
                    }
                    break;
            }
        };
        websocket.onclose = function (e) {
            websocket = null;
            Log("WebSocket is closed");
        }
    }
}

function SendUserInfo() {
    let UserId = document.getElementById('userid_id');
    let UserPasswd1 = document.getElementById('password_id_1');
    let UserPasswd2 = document.getElementById('password_id_2');
    let UserName = document.getElementById('name_id');
    let UserEmail = document.getElementById('email_id');
    let UserPhone = document.getElementById('phone_id');
    let UserAddress = document.getElementById('address_id');

    let emailRules = /^[A-Za-z0-9_\.\-]+@[A-Za-z0-9\-]+\.[A-Za-z0-9\-]+/;
    let passwordRules = /^(?=.*[a-zA-Z])(?=.*[!@#$%^*+=-])(?=.*[0-9]).{10,15}$/;
    let websocket = null;

    if (UserId.value.includes('|')) {
        Log("userid can NOT include '|' character");
        alert("userid can NOT include '|' character");
        return;
    }
    if (UserPasswd1.value.includes('|')) {
        Log("password can NOT include '|' character");
        alert("password can NOT include '|' character");
        return;
    }
    if (UserName.value.includes('|')) {
        Log("user name can NOT include '|' character");
        alert("user name can NOT include '|' character");
        return;
    }
    if (UserEmail.value.includes('|')) {
        Log("user email can NOT include '|' character");
        alert("user email can NOT include '|' character");
        return;
    }
    if (UserPhone.value.includes('|')) {
        Log("user phone can NOT include '|' character");
        alert("user phone can NOT include '|' character");
        return;
    }
    if (UserAddress.value.includes('|')) {
        Log("user address can NOT include '|' character");
        alert("user address can NOT include '|' character");
        return;
    }

    if (UserId.value.length == 0) {
        Log("userid has not been entered");
        alert("userid has not been entered");
        return;
    }

    if (UserPasswd1.value.length == 0) {
        Log("password has not been entered");
        alert("password has not been entered");
        return;
    }

    if (UserPasswd1.value != UserPasswd2.value) {
        Log("password entered is NOT same");
        alert("password entered is NOT same");
        return;
    }

    if (passwordRules.test(UserPasswd1.value) == false) {
        Log("Password format is not valid : " + UserPasswd1.value);
        alert("Password format is not valid");
        return;
    }

    if (UserName.value.length == 0) {
        Log("user name has not been entered");
        alert("user name has not been entered");
        return;
    }

    if (UserEmail.value.length == 0) {
        Log("Email has not been entered");
        alert("Email has not been entered");
        return;
    }

    if (emailRules.test(UserEmail.value) == false) {
        Log("Email format is not valid: " + UserEmail.value);
        alert("Email format is not valid");
        return;
    }

    if (UserPhone.value.length == 0) {
        UserPhone.value = " ";
    }

    if (UserAddress.value.length == 0) {
        UserAddress.value = " ";
    }

    if (websocket == null) {
        if (window.location.protocol == "https:") {
            websocket = new WebSocket("wss://" + window.location.hostname);
        }
        else {
            return;
        }

        websocket.onopen = function (e) {
            websocket.send("req|register|" +
                UserId.value + "|" +
                UserPasswd1.value + "|" +
                UserName.value + "|" +
                UserEmail.value + "|" +
                UserPhone.value + "|" +
                UserAddress.value)
        };
        websocket.onmessage = function (e) {
            Log("Recv[" + e.data + "]");

            var arrData = e.data.split("|");

            websocket.close();
            switch (arrData[0]) {
                case "res":
                    switch (arrData[1]) {
                        case "register":
                            if (arrData[2] == '200') {
                                alert("User registration was successful.[" + UserId.value + "]");

                                // display Otp Key
                                var txtOtpKey = document.getElementById('google_key');
                                txtOtpKey.value = arrData[3]

                                // display Otp QR code
                                var txtIssuer = document.getElementById('userid_id');
                                var issuer = txtIssuer.value;
                                var txtName = document.getElementById('name_id');
                                var accountName = txtName.value;
                                var txtKey = document.getElementById('google_key');
                                var secretKey = txtKey.value;

                                var otpAuthUrl = "otpauth://totp/" + issuer + ":" + accountName + "?secret=" + secretKey + "&issuer=" + issuer;

                                // Clear the previous QR code
                                var qrcodeElement = document.getElementById("qrcode");

                                while (qrcodeElement.firstChild) {
                                    qrcodeElement.firstChild.remove();
                                }

                                var qrcode = new QRCode(document.getElementById("qrcode"), {
                                    text: otpAuthUrl,
                                    width: 128,
                                    height: 128,
                                });
                            }
                            else {
                                var iStatusCode = parseInt(arrData[2]);

                                if (iStatusCode == 400) {
                                    alert("user ID is alread registered\r\nPlease use another user ID");
                                }
                                else if (iStatusCode == 410) {
                                    alert("can not INSERT the user info to mysql");
                                }
                                else if (iStatusCode == 420) {
                                    Log("Password format is not valid");
                                    alert("password format is not valid");
                                }
                            }
                            break;
                    }
                    break;
            }
        };
        websocket.onclose = function (e) {
            websocket = null;
            Log("WebSocket is closed");
        }
    }
}

function StartSession() {
    var txtUserId = document.getElementById('user_id');
    gstrUserId = txtUserId.value;

    var txtUserPasswd = document.getElementById('user_passwd');
    gstrUserPasswd = txtUserPasswd.value;

    var txtOtpNum = document.getElementById('otp_num');
    gstrOtpNum = txtOtpNum.value;

    let otpRules = /^[0-9]+$/;
    if (gstrUserId.length == 0) {
        alert("user id is not defined");
        return;
    }
    if (gstrUserPasswd.length == 0) {
        alert("password is not provided");
        return;
    }
    if (gstrOtpNum.length == 0) {
        alert("OTP is not provided");
        return;
    }
    if (otpRules.test(gstrOtpNum) == false) {
        alert("OTP should be number!");
        return;
    }

    if (ws == null) {
        if (window.location.protocol == "https:") {
            ws = new WebSocket("wss://" + window.location.hostname);
        }
        else {
            return;
        }

        ws.onopen = function (e) {
            Send("req|login|" + gstrUserId + "|" + gstrUserPasswd + "|" + gstrOtpNum);
        };

        ws.onmessage = function (e) {

            Log("Recv[" + e.data + "]");

            var arrData = e.data.split("|");

            switch (arrData[0]) {
                case "res":
                    switch (arrData[1]) {
                        case "login":
                            if (arrData[2] == '200') {
                                btnLogin.disabled = true;
                                btnRetreiveList.disabled = false;
                                btnChangeEmail.disabled = false;
                                btnChangePassword.disabled = false;
                                btnInvite.disabled = false;
                                btnResetPassword.disabled = true;
                            }
                            else {
                                var iStatusCode = parseInt(arrData[2]);

                                if (iStatusCode == 300) {
                                    alert("Unregisterd user! \r\nPlease register to use VideoChat");
                                }
                                else if (iStatusCode == 400) {
                                    alert("Password is WRONG!");
                                }
                                else if (iStatusCode == 410) {
                                    alert("Password Database failed");
                                }
                                else if (iStatusCode == 420) {
                                    alert("Too many password is wrong. \r\n Wait for 60 seconds...");
                                }
                                else if (iStatusCode == 430) {
                                    alert("Your password is outdated and needs to be changed.");
                                }
                                else if (iStatusCode == 440) {
                                    alert("OTP is WRONG!");
                                }

                                btnLogin.disabled = false;
                                btnResetPassword.disabled = false;
                                btnInvite.disabled = true;

                                ws = null;
                                InitButton();
                            }

                            break;
                        case "invite":
                            if (arrData[2] == '200') {
                                setAnswer(arrData[3]);

                                btnInvite.disabled = true;
                                btnAccept.disabled = true;
                                btnDecline.disabled = true;
                                btnBye.disabled = false;
                            }
                            else {
                                var iStatusCode = parseInt(arrData[2]);

                                if (iStatusCode == 404) {
                                    alert("peer id is NOT log-in now!\r\nPlease press \"Retreive logined ID\" button");
                                    btnInvite.disabled = false;
                                }
                                else if (iStatusCode == 500) {
                                    alert("peer id is ALREADY in the another call\r\nYou can connect after previous call is finished!");
                                    btnInvite.disabled = false;
                                }
                                else if (iStatusCode > 200) {
                                    btnInvite.disabled = false;
                                }
                            }
                            btnBye.disabled = false;
                            break;
                        case "contact":
                            ClearLog()
                            Print("### Contact ID Logined ###")
                            //Print("unique_id   /  name  /   email  /   phone  / address ")
                            var iContactCount = parseInt(arrData[2]);
                            for (var i = 0; i < iContactCount; i++) {
                                //Print(arrData[3+i])
                                // send userinfo
                                Send("req|userinfo|" + arrData[3 + i]);
                            }
                            break;
                        case "changepwd":
                            var iStatusCode = parseInt(arrData[2]);

                            if (iStatusCode == 200) {
                                alert("Password has been changed.");
                            }
                            else if (iStatusCode == 300) {
                                alert("Failed to save new password to DB");
                            }
                            break;
                        case "userinfo":
                            Print("[" + arrData[2] + "]  " + arrData[3] + " / " + arrData[4] + " / " + arrData[5] + " / " + arrData[6])
                            break;
                    }
                    break;

                case "req":
                    switch (arrData[1]) {
                        case "invite":
                            gstrToId = arrData[2];
                            gstrSdp = arrData[3];
                            var txtPeerId = document.getElementById('peer_id');
                            txtPeerId.value = gstrToId;

                            Log("Invite event peer(" + gstrToId + ") sdp(" + arrData[3] + ")");
                            alert('You have a call from ' + gstrToId + '\r\nPlease press Accept or Decline!');
                            //createAnswer( clsData.sdp );

                            btnInvite.disabled = true;
                            btnAccept.disabled = false;
                            btnDecline.disabled = false;
                            btnBye.disabled = true;
                            break;
                        case "bye":
                            if (btnAccept.disabled == false)
                                alert('You have a missedcall from ' + gstrToId);
                            else
                                alert(gstrToId + ' call ended');
                            gstrToId = "";
                            stopPeer();
                            btnInvite.disabled = false;
                            btnAccept.disabled = true;
                            btnDecline.disabled = true;
                            btnBye.disabled = true;
                            break;
                    }
            }
        };

        ws.onclose = function (e) {
            ws = null;
            InitButton();
            Log("WebSocket is closed");
        }
    }
}

function ContactList() {
    Send("req|contact");
}

function Send(data) {
    ws.send(data);
    Log("Send[" + data + "]");
}

function Invite(strSdp) {
    Send("req|invite|" + gstrToId + "|" + strSdp);
}

function Accept(strSdp) {
    Send("res|invite|200|" + strSdp);
}

function SendInvite() {
    var txtPeerId = document.getElementById('peer_id');
    var strPeerId = txtPeerId.value;

    if (strPeerId.length == 0) {
        alert('peer id is not defined');
        return;
    }

    btnInvite.disabled = true;
    btnAccept.disabled = true;
    btnDecline.disabled = true;
    btnBye.disabled = true;

    gstrToId = strPeerId;
    createOffer();

    //Invite( "o=" + strPeerId );
}

function SendAccept() {
    btnInvite.disabled = true;
    btnAccept.disabled = true;
    btnDecline.disabled = true;
    btnBye.disabled = false;

    createAnswer(gstrSdp);
    //Accept( "o=accept" );
}

function SendDecline() {
    Send("res|invite|603");

    btnInvite.disabled = false;
    btnAccept.disabled = true;
    btnDecline.disabled = true;
}

function SendBye() {
    stopPeer();
    Send("req|bye");
    gstrToId = "";
    btnInvite.disabled = false;
    btnBye.disabled = true;
}

function changeEmail() {
    var emailModal = document.getElementById("emailModal");
    emailModal.style.display = "block";
}

document.getElementById("emailForm").addEventListener("submit", function (event) {
    event.preventDefault();

    var newEmail = document.getElementById("new_email").value;

    document.getElementById("emailModal").style.display = "none";

    if (newEmail.includes('|')) {
        Log("user email can NOT include '|' character");
        alert("user email can NOT include '|' character");
        return;
    }

    let emailRules = /^[A-Za-z0-9_\.\-]+@[A-Za-z0-9\-]+\.[A-Za-z0-9\-]+/;
    if (newEmail.length == 0) {
        Log("Email has not been entered");
        alert("Email has not been entered");
        return;
    }

    if (emailRules.test(newEmail) == false) {
        Log("Email format is not valid: " + newEmail);
        alert("Email format is not valid");
        return;
    }

    Send("req|changeacc|" + gstrUserId + "|" + newEmail);
});

var emailModal = document.getElementById("emailModal");
var emailSpan = document.getElementsByClassName("close")[0];

btnChangeEmail.onclick = function () {
    emailModal.style.display = "block";
}

emailSpan.onclick = function () {
    emailModal.style.display = "none";
}

window.onclick = function (event) {
    if (event.target == emailModal) {
        emailModal.style.display = "none";
    }
}

function changePassword() {
    var pwdmodal = document.getElementById("passwordModal");
    pwdmodal.style.display = "block";
}

document.getElementById("passwordForm").addEventListener("submit", function (event) {
    event.preventDefault();

    var newPassword = document.getElementById("new_password").value;

    document.getElementById("passwordModal").style.display = "none";

    if (newPassword.includes('|')) {
        Log("password can NOT include '|' character");
        alert("password can NOT include '|' character");
        return;
    }

    let passwordRules = /^(?=.*[a-zA-Z])(?=.*[!@#$%^*+=-])(?=.*[0-9]).{10,15}$/;

    if (newPassword.length == 0) {
        Log("Password has not been entered");
        alert("Password has not been entered");
        return;
    }

    if (passwordRules.test(newPassword) == false) {
        Log("Password format is not valid: " + newPassword);
        alert("Password format is not valid");
        return;
    }

    Send("req|changepwd|" + gstrUserId + "|" + newPassword);
});

var pwdmodal = document.getElementById("passwordModal");
var pwdspan = document.getElementsByClassName("close")[1];

btnChangePassword.onclick = function () {
    pwdmodal.style.display = "block";
}

pwdspan.onclick = function () {
    pwdmodal.style.display = "none";
}

window.onclick = function (event) {
    if (event.target == pwdmodal) {
        pwdmodal.style.display = "none";
    }
}

function resetPassword() {
    var resetModal = document.getElementById("resetModal");
    resetModal.style.display = "block";
}

document.getElementById("resetForm").addEventListener("submit", function (event) {
    event.preventDefault();

    var resetID = document.getElementById("reset_id").value;
    var resetOTP = document.getElementById("reset_otp").value;

    document.getElementById("resetModal").style.display = "none";

    if (resetID.includes('|')) {
        Log("userid can NOT include '|' character");
        alert("userid can NOT include '|' character");
        return;
    }

    if (resetID.length == 0) {
        Log("userid has not been entered");
        alert("userid has not been entered");
        return;
    }

    if (resetOTP.length == 0) {
        alert("OTP is not provided");
        return;
    }

    let otpRules = /^[0-9]+$/;
    if (otpRules.test(resetOTP) == false) {
        alert("OTP should be number!");
        return;
    }

    if (ws == null) {
        ws = new WebSocket("wss://" + window.location.hostname);
        ws.onopen = function (e) {
            Send("req|resetpwd|" + resetID + "|" + resetOTP);
        };

        ws.onmessage = function (e) {
            Log("Recv[" + e.data + "]");
            var arrData = e.data.split("|");

            switch (arrData[0]) {
                case "res":
                    switch (arrData[1]) {
                        case "resetpwd":
                            var iStatusCode = parseInt(arrData[2]);
                            if (iStatusCode == 200) {
                                alert("Password has been reset. Please check your email");
                            }
                            else if (iStatusCode == 300) {
                                alert("Failed to send email");
                            }
                            else if (iStatusCode == 400) {
                                alert("Failed to save new password to DB");
                            }
                            else if (iStatusCode == 500) {
                                alert("Unregisterd user");
                            }
                            else if (iStatusCode == 510) {
                                alert("Wrong OTP number");
                            }
                            break;
                        default:
                            ws = null;
                            break;
                    }
                default:
                    break;
            }
        };
    }

    ws.onclose = function (e) {
        ws = null;
        Log("WebSocket is closed");
    }
});

var resetModal = document.getElementById("resetModal");
var resetSpan = document.getElementsByClassName("close")[2];

btnResetPassword.onclick = function () {
    resetModal.style.display = "block";
}

resetSpan.onclick = function () {
    resetModal.style.display = "none";
}

window.onclick = function (event) {
    if (event.target == resetModal) {
        resetModal.style.display = "none";
    }
}