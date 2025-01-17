#include "StreamSettingsWidget.hpp"

#include "ui/widgets/common/WidgetUIBase.hpp"
#include "ui/widgets/editors/w_ChainSha256Editor.hpp"
#include "ui/widgets/editors/w_JsonEditor.hpp"
#include "utils/QvHelpers.hpp"

#define QV_MODULE_NAME "StreamSettingsWidget"

StreamSettingsWidget::StreamSettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
    QvMessageBusConnect(StreamSettingsWidget);
}

QvMessageBusSlotImpl(StreamSettingsWidget)
{
    switch (msg)
    {
        MBRetranslateDefaultImpl;
        case UPDATE_COLORSCHEME:
        case HIDE_WINDOWS:
        case SHOW_WINDOWS: break;
    }
}

StreamSettingsObject StreamSettingsWidget::GetStreamSettings() const
{
    return stream;
}

void StreamSettingsWidget::SetStreamObject(const StreamSettingsObject &sso)
{
    stream = sso;
    transportCombo->setCurrentText(stream.network);
    // TLS XTLS Reality
    {
        const static QMap<QString, int> securityIndexMap{ { "none", 0 }, { "tls", 1 }, {"reality", 2}, { "xtls", 3 } };
        const static QMap<QString, int> fingerprintIndexMap{ { "", 0 }, { "chrome", 1}, { "firefox", 2}, {"safari", 3}, {"ios", 4}, {"android", 5}, {"edge", 6}, {"randomized", 7}};

        if (securityIndexMap.contains(stream.security))
            securityTypeCB->setCurrentIndex(securityIndexMap[stream.security]);
        else
            LOG("Unsupported Security Type:", stream.security);


        #define prefix stream.security
        if (stream.security == "tls" )
        {
            serverNameTxt->setText(stream.tlsSettings.serverName);  
            allowInsecureCB->setChecked(stream.tlsSettings.allowInsecure);
            enableSessionResumptionCB->setChecked(stream.tlsSettings.enableSessionResumption);
            disableSystemRoot->setChecked(stream.tlsSettings.disableSystemRoot);
            alpnH2CB->setChecked(stream.tlsSettings.alpn.contains("h2"));
            alpnHttpCB->setChecked(stream.tlsSettings.alpn.contains("http/1.1"));
            if (fingerprintIndexMap.contains(stream.tlsSettings.fingerprint))
                fingerprintCB->setCurrentIndex(fingerprintIndexMap[stream.tlsSettings.fingerprint]);
            else
                LOG("Unsupported fingerprint value:", stream.tlsSettings.fingerprint);
        }
        else if (stream.security == "xtls" )
        {
            serverNameTxt->setText(stream.xtlsSettings.serverName);  
            allowInsecureCB->setChecked(stream.xtlsSettings.allowInsecure);
            enableSessionResumptionCB->setChecked(stream.xtlsSettings.enableSessionResumption);
            disableSystemRoot->setChecked(stream.xtlsSettings.disableSystemRoot);
            alpnH2CB->setChecked(stream.xtlsSettings.alpn.contains("h2"));
            alpnHttpCB->setChecked(stream.xtlsSettings.alpn.contains("http/1.1"));
            if (fingerprintIndexMap.contains(stream.xtlsSettings.fingerprint))
                fingerprintCB->setCurrentIndex(fingerprintIndexMap[stream.xtlsSettings.fingerprint]);
            else
                LOG("Unsupported fingerprint value:", stream.xtlsSettings.fingerprint);
        }
        else if (prefix == "reality")
        {
            serverNameTxt->setText(stream.realitySettings.serverName);  
            publicKeyTxt->setText(stream.realitySettings.publicKey);
            shortIdTxt->setText(stream.realitySettings.shortId);
            spiderXTxt->setText(stream.realitySettings.spiderX);
            if (fingerprintIndexMap.contains(stream.realitySettings.fingerprint))
                fingerprintCB->setCurrentIndex(fingerprintIndexMap[stream.realitySettings.fingerprint]);
            else
                LOG("Unsupported fingerprint value:", stream.realitySettings.fingerprint);
        }
    }

    // TCP
    {
        tcpHeaderTypeCB->setCurrentText(stream.tcpSettings.header.type);
        tcpRequestTxt->setPlainText(JsonToString(stream.tcpSettings.header.request.toJson()));
        tcpRespTxt->setPlainText(JsonToString(stream.tcpSettings.header.response.toJson()));
    }
    // HTTP
    {
        httpHostTxt->setPlainText(stream.httpSettings.host.join(NEWLINE));
        httpPathTxt->setText(stream.httpSettings.path);
        httpMethodCB->setCurrentText(stream.httpSettings.method);
        httpHeadersTxt->setPlainText(JsonToString(stream.httpSettings.toJson()["headers"].toObject()));
    }
    // WS
    {
        wsPathTxt->setText(stream.wsSettings.path);
        QString wsHeaders;
        for (const auto &[key, value] : stream.wsSettings.headers.toStdMap())
        {
            wsHeaders = wsHeaders % key % "|" % value % NEWLINE;
        }
        wsHeadersTxt->setPlainText(wsHeaders);
        wsEarlyDataSB->setValue(stream.wsSettings.maxEarlyData);
        wsBrowserForwardCB->setChecked(stream.wsSettings.useBrowserForwarding);
        wsEarlyDataHeaderNameCB->setCurrentText(stream.wsSettings.earlyDataHeaderName);
    }
    // mKCP
    {
        kcpMTU->setValue(stream.kcpSettings.mtu);
        kcpTTI->setValue(stream.kcpSettings.tti);
        kcpHeaderType->setCurrentText(stream.kcpSettings.header.type);
        kcpCongestionCB->setChecked(stream.kcpSettings.congestion);
        kcpReadBufferSB->setValue(stream.kcpSettings.readBufferSize);
        kcpUploadCapacSB->setValue(stream.kcpSettings.uplinkCapacity);
        kcpDownCapacitySB->setValue(stream.kcpSettings.downlinkCapacity);
        kcpWriteBufferSB->setValue(stream.kcpSettings.writeBufferSize);
        kcpSeedTxt->setText(stream.kcpSettings.seed);
    }
    // DS
    {
        dsPathTxt->setText(stream.dsSettings.path);
    }
    // QUIC
    {
        quicKeyTxt->setText(stream.quicSettings.key);
        quicSecurityCB->setCurrentText(stream.quicSettings.security);
        quicHeaderTypeCB->setCurrentText(stream.quicSettings.header.type);
    }
    // gRPC
    {
        grpcServiceNameTxt->setText(stream.grpcSettings.serviceName);
        grpcModeCB->setCurrentText(stream.grpcSettings.multiMode ? "multi" : "gun");
    }
    // SOCKOPT
    {
        tProxyCB->setCurrentText(stream.sockopt.tproxy);
        tcpFastOpenCB->setChecked(stream.sockopt.tcpFastOpen);
        soMarkSpinBox->setValue(stream.sockopt.mark);
        tcpKeepAliveIntervalSpinBox->setValue(stream.sockopt.tcpKeepAliveInterval);
    }
}

void StreamSettingsWidget::on_httpPathTxt_textEdited(const QString &arg1)
{
    stream.httpSettings.path = arg1;
}

void StreamSettingsWidget::on_httpHostTxt_textChanged()
{
    const auto hosts = httpHostTxt->toPlainText().replace("\r", "").split("\n");
    stream.httpSettings.host.clear();
    for (const auto &host : hosts)
    {
        if (!host.trimmed().isEmpty())
            stream.httpSettings.host.push_back(host.trimmed());
    }
}

void StreamSettingsWidget::on_wsHeadersTxt_textChanged()
{
    const auto headers = SplitLines(wsHeadersTxt->toPlainText());
    stream.wsSettings.headers.clear();
    for (const auto &header : headers)
    {
        if (header.isEmpty())
            continue;

        if (!header.contains("|"))
        {
            LOG("Header missing '|' separator");
            RED(wsHeadersTxt);
            return;
        }

        const auto index = header.indexOf("|");
        auto key = header.left(index);
        auto value = header.right(header.length() - index - 1);
        stream.wsSettings.headers[key] = value;
    }
    BLACK(wsHeadersTxt);
}

void StreamSettingsWidget::on_tcpRequestDefBtn_clicked()
{
    tcpRequestTxt->clear();
    tcpRequestTxt->setPlainText(JsonToString(HTTPRequestObject().toJson()));
    stream.tcpSettings.header.request = HTTPRequestObject();
}

void StreamSettingsWidget::on_tcpRespDefBtn_clicked()
{
    tcpRespTxt->clear();
    tcpRespTxt->setPlainText(JsonToString(HTTPResponseObject().toJson()));
    stream.tcpSettings.header.response = HTTPResponseObject();
}

void StreamSettingsWidget::on_soMarkSpinBox_valueChanged(int arg1)
{
    stream.sockopt.mark = arg1;
}

void StreamSettingsWidget::on_tcpFastOpenCB_stateChanged(int arg1)
{
    stream.sockopt.tcpFastOpen = arg1 == Qt::Checked;
}

void StreamSettingsWidget::on_tProxyCB_currentIndexChanged(int arg1)
{
    stream.sockopt.tproxy = tProxyCB->itemText(arg1);
}

void StreamSettingsWidget::on_quicSecurityCB_currentIndexChanged(int arg1)
{
    stream.quicSettings.security = quicSecurityCB->itemText(arg1);
}

void StreamSettingsWidget::on_quicKeyTxt_textEdited(const QString &arg1)
{
    stream.quicSettings.key = arg1;
}

void StreamSettingsWidget::on_quicHeaderTypeCB_currentIndexChanged(int arg1)
{
    stream.quicSettings.header.type = quicHeaderTypeCB->itemText(arg1);
}

void StreamSettingsWidget::on_tcpHeaderTypeCB_currentIndexChanged(int arg1)
{
    stream.tcpSettings.header.type = tcpHeaderTypeCB->itemText(arg1);
}

void StreamSettingsWidget::on_wsPathTxt_textEdited(const QString &arg1)
{
    stream.wsSettings.path = arg1;
}

void StreamSettingsWidget::on_kcpMTU_valueChanged(int arg1)
{
    stream.kcpSettings.mtu = arg1;
}

void StreamSettingsWidget::on_kcpTTI_valueChanged(int arg1)
{
    stream.kcpSettings.tti = arg1;
}

void StreamSettingsWidget::on_kcpUploadCapacSB_valueChanged(int arg1)
{
    stream.kcpSettings.uplinkCapacity = arg1;
}

void StreamSettingsWidget::on_kcpCongestionCB_stateChanged(int arg1)
{
    stream.kcpSettings.congestion = arg1 == Qt::Checked;
}

void StreamSettingsWidget::on_kcpDownCapacitySB_valueChanged(int arg1)
{
    stream.kcpSettings.downlinkCapacity = arg1;
}

void StreamSettingsWidget::on_kcpReadBufferSB_valueChanged(int arg1)
{
    stream.kcpSettings.readBufferSize = arg1;
}

void StreamSettingsWidget::on_kcpWriteBufferSB_valueChanged(int arg1)
{
    stream.kcpSettings.writeBufferSize = arg1;
}

void StreamSettingsWidget::on_kcpHeaderType_currentIndexChanged(int arg1)
{
    stream.kcpSettings.header.type = kcpHeaderType->itemText(arg1);
}

void StreamSettingsWidget::on_kcpSeedTxt_textEdited(const QString &arg1)
{
    stream.kcpSettings.seed = arg1;
}

void StreamSettingsWidget::on_dsPathTxt_textEdited(const QString &arg1)
{
    stream.dsSettings.path = arg1;
}

void StreamSettingsWidget::on_tcpRequestEditBtn_clicked()
{
    JsonEditor w(JsonFromString(tcpRequestTxt->toPlainText()), this);
    auto rJson = w.OpenEditor();
    tcpRequestTxt->setPlainText(JsonToString(rJson));
    auto tcpReqObject = HTTPRequestObject::fromJson(rJson);
    stream.tcpSettings.header.request = tcpReqObject;
}

void StreamSettingsWidget::on_tcpResponseEditBtn_clicked()
{
    JsonEditor w(JsonFromString(tcpRespTxt->toPlainText()), this);
    auto rJson = w.OpenEditor();
    tcpRespTxt->setPlainText(JsonToString(rJson));
    auto tcpRspObject = HTTPResponseObject::fromJson(rJson);
    stream.tcpSettings.header.response = tcpRspObject;
}

void StreamSettingsWidget::on_transportCombo_currentIndexChanged(int arg1)
{
    stream.network = transportCombo->itemText(arg1);
    v2rayStackView->setCurrentIndex(arg1);
}

void StreamSettingsWidget::on_securityTypeCB_currentIndexChanged(int arg1)
{
    stream.security = securityTypeCB->itemText(arg1).toLower();
    if (stream.security == "reality")
    {
        allowInsecureCB->setEnabled(false);
        enableSessionResumptionCB->setEnabled(false);
        disableSystemRoot->setEnabled(false);
        labelALPN->setEnabled(false);
        alpnH2CB->setEnabled(false);
        alpnHttpCB->setEnabled(false);
        labelPublicKey->setEnabled(true);
        publicKeyTxt->setEnabled(true);
        labelShortId->setEnabled(true);
        shortIdTxt->setEnabled(true);
        labelSpiderX->setEnabled(true);
        spiderXTxt->setEnabled(true);
    }
    else
    {
        allowInsecureCB->setEnabled(true);
        enableSessionResumptionCB->setEnabled(true);
        disableSystemRoot->setEnabled(true);
        labelALPN->setEnabled(true);
        alpnH2CB->setEnabled(true);
        alpnHttpCB->setEnabled(true);
        labelPublicKey->setEnabled(false);
        publicKeyTxt->setEnabled(false);
        labelShortId->setEnabled(false);
        shortIdTxt->setEnabled(false);
        labelSpiderX->setEnabled(false);
        spiderXTxt->setEnabled(false);
    }
}

void StreamSettingsWidget::on_fingerprintCB_currentIndexChanged(int arg1)
{
    stream.tlsSettings.fingerprint = fingerprintCB->itemText(arg1).toLower();
    stream.xtlsSettings.fingerprint = fingerprintCB->itemText(arg1).toLower();
    stream.realitySettings.fingerprint = fingerprintCB->itemText(arg1).toLower();
}

//
// Dirty hack, since XTLSSettings are the same as TLSSettings (Split them if required in the future)
//
void StreamSettingsWidget::on_serverNameTxt_textEdited(const QString &arg1)
{
    stream.tlsSettings.serverName = arg1.trimmed();
    stream.xtlsSettings.serverName = arg1.trimmed();
    stream.realitySettings.serverName = arg1.trimmed();
}

void StreamSettingsWidget::on_allowInsecureCB_stateChanged(int arg1)
{
    stream.tlsSettings.allowInsecure = arg1 == Qt::Checked;
    stream.xtlsSettings.allowInsecure = arg1 == Qt::Checked;
}

void StreamSettingsWidget::on_publicKeyTxt_textEdited(const QString &arg1)
{
    stream.realitySettings.publicKey = arg1.trimmed();
}

void StreamSettingsWidget::on_shortIdTxt_textEdited(const QString &arg1)
{
    stream.realitySettings.shortId = arg1.trimmed();
}
void StreamSettingsWidget::on_spiderXTxt_textEdited(const QString &arg1)
{
    stream.realitySettings.spiderX = arg1.trimmed();
}

void StreamSettingsWidget::on_alpnH2CB_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        if (!stream.tlsSettings.alpn.contains("h2"))
            stream.tlsSettings.alpn.insert(0,"h2");
        if (!stream.xtlsSettings.alpn.contains("h2"))
            stream.xtlsSettings.alpn.insert(0,"h2");

    } else {
        stream.tlsSettings.alpn.removeAll("h2");
        stream.xtlsSettings.alpn.removeAll("h2");
    }
}

void StreamSettingsWidget::on_alpnHttpCB_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        if (!stream.tlsSettings.alpn.contains("http/1.1"))
            stream.tlsSettings.alpn.append("http/1.1");
        if (!stream.xtlsSettings.alpn.contains("http/1.1"))
            stream.xtlsSettings.alpn.append("http/1.1");

    } else {
        stream.tlsSettings.alpn.removeAll("http/1.1");
        stream.xtlsSettings.alpn.removeAll("http/1.1");
    }
}

void StreamSettingsWidget::on_enableSessionResumptionCB_stateChanged(int arg1)
{
    stream.tlsSettings.enableSessionResumption = arg1 == Qt::Checked;
    stream.xtlsSettings.enableSessionResumption = arg1 == Qt::Checked;
}

void StreamSettingsWidget::on_disableSystemRoot_stateChanged(int arg1)
{
    stream.tlsSettings.disableSystemRoot = arg1;
    stream.xtlsSettings.disableSystemRoot = arg1;
}

void StreamSettingsWidget::on_openCertEditorBtn_clicked()
{
}

void StreamSettingsWidget::on_grpcServiceNameTxt_textEdited(const QString &arg1)
{
    stream.grpcSettings.serviceName = arg1;
}

void StreamSettingsWidget::on_grpcModeCB_currentIndexChanged(int arg1)
{
    stream.grpcSettings.multiMode = grpcModeCB->itemText(arg1).toLower() == "multi";
}

void StreamSettingsWidget::on_wsEarlyDataSB_valueChanged(int arg1)
{
    stream.wsSettings.maxEarlyData = arg1;
}

void StreamSettingsWidget::on_wsBrowserForwardCB_stateChanged(int arg1)
{
    stream.wsSettings.useBrowserForwarding = arg1 == Qt::Checked;
}

void StreamSettingsWidget::on_pinnedPeerCertificateChainSha256Btn_clicked()
{
    ChainSha256Editor ed(this, stream.tlsSettings.pinnedPeerCertificateChainSha256);
    if (ed.exec() == QDialog::Accepted)
    {
        stream.tlsSettings.pinnedPeerCertificateChainSha256 = QList<QString>(ed);
    }
}

void StreamSettingsWidget::on_wsEarlyDataHeaderNameCB_currentIndexChanged(int arg1)
{
    stream.wsSettings.earlyDataHeaderName = wsEarlyDataHeaderNameCB->itemText(arg1);
}

void StreamSettingsWidget::on_httpMethodCB_currentTextChanged(const QString &arg1)
{
    stream.httpSettings.method = arg1;
}

void StreamSettingsWidget::on_tcpKeepAliveIntervalSpinBox_valueChanged(int arg1)
{
    stream.sockopt.tcpKeepAliveInterval = arg1;
}

void StreamSettingsWidget::on_httpHeadersDefBtn_clicked()
{
    httpHeadersTxt->clear();
    httpHeadersTxt->setPlainText(JsonToString(HttpObject().toJson()["headers"].toObject()));
    stream.httpSettings.headers = HttpObject().headers;
}

void StreamSettingsWidget::on_httpHeadersEditBtn_clicked()
{
    JsonEditor w(JsonFromString(httpHeadersTxt->toPlainText()), this);
    auto rJson = w.OpenEditor();
    httpHeadersTxt->setPlainText(JsonToString(rJson));

    auto json = HttpObject().toJson();
    json["headers"] = rJson;
    stream.httpSettings.headers = HttpObject::fromJson(json).headers;
}
