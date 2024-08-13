// index.js
const defaultAvatarUrl = 'https://mmbiz.qpic.cn/mmbiz/icTdbqWNOwNRna42FI242Lcia07jQodd2FJGIYQfG0LAJGFxM4FbnQP6yfMxBgJ0F3YRqJCJ1aPAK2dQagdusBZg/0'

// 地图天气api
const KEY = "c4f1544acff3ed270c12e52724ccfb01";

const app = getApp();

import mqtt from '../../utils/mqtt.js';
const aliyunOpt = require('../../utils/aliyun/aliyun_connect.js')

Page({
    //系统声明的变量
    data: {
        title: 'RTT Sensor HubApp',
        welcome: '欢迎使用！天气不错哦',
        location: '无',
        temperature: 0,
        weatherIcon: '',
        weatherText: '',
        isMQTTSeverConnect: false,
        isTopicSubscribe: false,
        isTopicModfiy: false,
        mqttConnectDiolog: false, //mqtt连接弹窗是否打开

        // LED矩阵模式选择
        ledn_select_value: 0,
        ledn_isOpen: 0,
        ledn_option1: [{
                text: '模式1 彩虹环闪',
                value: 0
            },
            {
                text: '模式2 环形变绿',
                value: 1
            },
            {
                text: '模式3 环形变红',
                value: 2
            },
            {
                text: '模式4 绿红蓝爆闪',
                value: 3
            },
            {
                text: '模式5 彩虹微动',
                value: 4
            },
            {
                text: '模式6 白光呼吸',
                value: 5
            },
            {
                text: '模式7 静谧炫彩',
                value: 6
            },
            {
                text: '模式8 阴影旋转',
                value: 7
            },
        ],

        sensorList: [{
                img: "/images/P1.png",
                name: "AHT10",
                parameter: "温度",
                value: 0,
                unit: "°C",
                isPass: false,
                idx: 0
            },
            {
                img: "/images/P2.png",
                name: "AHT10",
                parameter: "湿度",
                value: 0,
                unit: "%rh",
                isPass: false,
                idx: 1
            },
            {
                img: "/images/P3.png",
                name: "AP3216C",
                parameter: "光强",
                value: 0,
                unit: "lux",
                isPass: false,
                idx: 2
            },
            {
                img: "/images/distanceSensor.png",
                name: "AP3216C",
                parameter: "接近感应",
                value: 0,
                unit: "",
                isPass: false,
                idx: 3
            }
        ],
        otherSensorList: [{
                img: "/images/redled.png",
                name: "红色LED",
                label: "redled",
                isOpen: false
            },
            {
                img: "/images/blueled.png",
                name: "蓝色LED",
                label: "blueled",
                isOpen: false
            },
            {
                img: "/images/beep.png",
                name: "蜂鸣器",
                label: "beep",
                isOpen: false
            },
        ],

        //mqtt服务器
        client: null,
        reconnectCounts: 0,

        //MQTT连接的配置
        options: {
            protocolVersion: 3, //MQTT连接协议版本
            clean: false,
            reconnectPeriod: 1000, //1000毫秒，两次重新连接之间的间隔
            connectTimeout: 30 * 1000, //1000毫秒，两次重新连接之间的间隔
            resubscribe: true, //如果连接断开并重新连接，则会再次自动订阅已订阅的主题（默认true）
            clientId: '',
            password: '',
            username: '',
        },

        productKey: wx.getStorageSync('productKey') || '',
        deviceName: wx.getStorageSync('deviceName') || '',
        deviceSecret: wx.getStorageSync('deviceSecret') || '',
        regionId: 'cn-shanghai',

        topicGet: wx.getStorageSync('topicGet') || '',
        topicPost: wx.getStorageSync('topicPost') || '',
    },
    // 系统启动后自动运行
    onLoad() {
        this.checkLocationAuth();
    },

    // 打开连接弹窗
    openDialog() {
        this.setData({
            mqttConnectDiolog: true
        });
    },
    // 关闭连接弹窗
    closeDialog() {
        this.setData({
            mqttConnectDiolog: false
        });
    },


    // 检查是否授权地理位置信息
    checkLocationAuth() {
        let that = this
        wx.getSetting({
            success: (res) => {
                let authSetting = res.authSetting
                if (authSetting['scope.userLocation']) {
                    // console.log("地理位置授权成功");
                    that.getCurrentLocation();
                } else if (authSetting['scope.userLocation'] === false) {
                    wx.showModal({
                        title: '您未开启地理位置授权',
                        content: '请在系统设置中打开位置授权，以便我们为您提供更好的服务',
                        success: (res) => {
                            if (res.confirm) {
                                wx.openSetting()
                            }
                        }
                    })
                } else {
                    wx.authorize({
                        scope: 'scope.userLocation',
                        success: () => {
                            that.getCurrentLocation()
                        },
                        fail: () => {
                            wx.showModal({
                                title: '您未开启地理位置授权',
                                content: '请在系统设置中打开位置授权，以便我们为您提供更好的服务',
                                success: (res) => {
                                    if (res.confirm) {
                                        wx.openSetting()
                                    }
                                }
                            })
                        }
                    })
                }
            }
        })
    },

    getWeatherIcon(weatherText) {
        switch (weatherText) {
            case "雷":
                return "/images/weather/lei.png";
            case "多云":
                return "/images/weather/duoyun.png";
            case "晴":
                return "/images/weather/qing.png";
            case "雪":
                return "/images/weather/xue.png";
            case "大雪":
                return "/images/weather/xue.png";
            case "中雪":
                return "/images/weather/xue.png";
            case "小雪":
                return "/images/weather/xue.png";
            case "雨":
                return "/images/weather/yu.png";
            case "大雨":
                return "/images/weather/yu.png";
            case "中雨":
                return "/images/weather/yu.png";
            case "小雨":
                return "/images/weather/yu.png";
            case "冰雹":
                return "/images/weather/bingbao.png";
            default:
                return "/images/weather/qing.png"; // 默认图标
        }
    },
    // 获取当前位置信息
    getCurrentLocation() {
        let that = this;
        // console.log("开始获取地理位置");
        wx.getLocation({
            type: 'gcj02', // 比较精确
            success: (res) => {
                // console.log("经纬度", res);
                if (res.errMsg === "getLocation:ok") {
                    wx.request({
                        url: "https://restapi.amap.com/v3/geocode/regeo",
                        data: {
                            key: KEY,
                            location: res.longitude + "," + res.latitude,
                        },
                        header: {
                            "content-type": "application/json",
                        },
                        success: function (geoRes) {
                            // console.log("坐标转换和查询天气", geoRes.data);
                            wx.setStorageSync('city', geoRes.data.regeocode.addressComponent.adcode);
                            that.setData({
                                location: geoRes.data.regeocode.addressComponent.province +
                                    " " +
                                    geoRes.data.regeocode.addressComponent.district,
                            });

                            wx.request({
                                url: "https://restapi.amap.com/v3/weather/weatherInfo",
                                data: {
                                    key: KEY,
                                    city: geoRes.data.regeocode.addressComponent.adcode,
                                },
                                header: {
                                    "content-type": "application/json",
                                },
                                success: function (weatherRes) {
                                    // console.log("天气", weatherRes.data);
                                    let weatherText = weatherRes.data.lives[0].weather;
                                    let weatherIcon = that.getWeatherIcon(weatherText);
                                    that.setData({
                                        temperature: weatherRes.data.lives[0].temperature,
                                        weatherText: weatherText,
                                        weatherIcon: weatherIcon,
                                        welcome: "欢迎使用！ 今天的天气是" + weatherRes.data.lives[0].weather,
                                    });
                                },
                            });
                        },
                        fail: function (err) {
                            console.log("获取地理位置失败", err);
                        }
                    });
                }
            },
            fail: (err) => {
                console.log("获取地理位置失败", err);
                wx.showModal({
                    title: '获取地理位置失败',
                    content: '请确保您已授权并打开了定位服务',
                });
            }
        });
    },

    // 连接服务器
    connectServer: function () {
        wx.setStorageSync('productKey', this.data.productKey);
        wx.setStorageSync('deviceName', this.data.deviceName);
        wx.setStorageSync('deviceSecret', this.data.deviceSecret);

        var that = this;
        let clientOpt = aliyunOpt.getAliyunIotMqttClient({
            productKey: that.data.productKey,
            deviceName: that.data.deviceName,
            deviceSecret: that.data.deviceSecret,
            regionId: that.data.regionId,
            port: that.data.port,
        });
        let host = 'wxs://' + clientOpt.host;
        this.setData({
            'options.clientId': clientOpt.clientId,
            'options.password': clientOpt.password,
            'options.username': clientOpt.username,
        });
        console.log("this.data.options host:" + host);
        console.log("this.data.options data:" + JSON.stringify(this.data.options));
        this.data.client = mqtt.connect(host, this.data.options);
        this.data.client.on('connect', function (connack) {
            if (that.data.reconnectCounts === 0) {

                wx.showToast({
                    title: '连接成功'
                });
            } else {
                wx.showToast({
                    title: '重新连接成功',
                    icon: 'none',
                    duration: 1000
                });
            }
            that.setData({
                reconnectCounts: 0,
                isMQTTSeverConnect: true,
            });

        });
        this.data.client.on("message", this.handleMessage.bind(this));
        this.data.client.on("error", function (error) {
            console.log("服务器 error 的回调" + error);
        });
        this.data.client.on("reconnect", function () {
            console.log("服务器 reconnect的回调");
            that.setData({
                reconnectCounts: that.data.reconnectCounts + 1
            });
        });
        this.data.client.on("offline", function (errr) {
            console.log("服务器offline的回调");
            that.setData({
                isMQTTSeverConnect: false
            });
        });
    },

    // 断开服务器
    disconnectServer: function () {
        if (this.data.client && this.data.client.connected) {
            this.data.client.end();
            this.setData({
                isMQTTSeverConnect: false,
                isTopicSubscribe: false,
                isTopicModify: false,
            });
            wx.showToast({
                title: '断开连接'
            });
        } else {
            wx.showToast({
                title: '服务器未连接',
                icon: 'none',
                duration: 2000
            });
        }
    },

    // 订阅主题
    subscribeTopic: function (topic) {
        var that = this;
        if (this.data.client && this.data.client.connected) {
            this.data.client.subscribe(topic, function (err, granted) {
                if (!err) {
                    wx.showToast({
                        title: '订阅主题成功'
                    });
                    wx.setStorageSync('subscribedTopic', topic);
                    that.setData({
                        isTopicSubscribe: true
                    });
                } else {
                    that.setData({
                        isTopicSubscribe: false,
                    });
                    wx.showToast({
                        title: '订阅主题失败',
                        icon: 'fail',
                        duration: 2000
                    });
                }
            });
        } else {
            wx.showToast({
                title: '请先连接服务器',
                icon: 'none',
                duration: 2000
            });
        }
    },

    // 取消订阅主题
    unsubscribeTopic: function (topic) {
        if (this.data.client && this.data.client.connected) {
            this.data.client.unsubscribe(topic);
            this.setData({
                isTopicSubscribe: false
            });
            wx.showToast({
                title: '取消订阅成功'
            });
        } else {
            wx.showToast({
                title: '请先连接服务器',
                icon: 'none',
                duration: 2000
            });
        }
    },

    // 发布消息
    publishTopic: function (topic, message) {
        let transdata={cmd:message}
            
        console.log(JSON.stringify(transdata));
        if (this.data.client && this.data.client.connected) {
            this.data.client.publish(topic, JSON.stringify(transdata));
            wx.showToast({
                title: '发布成功'
            });
        } else {
            wx.showToast({
                title: '请先连接服务器',
                icon: 'none',
                duration: 800
            });
        }
    },

    // 处理接收到的消息
    handleMessage: function (topic, payload) {
        // console.log("收到 topic:" + topic + " , payload :" + payload);
        let getMessageObj = {};
        getMessageObj = JSON.parse(payload);
        // console.log(getMessageObj);

        // 检查接收到的数据是否包含期望的字段
        if (!getMessageObj || !getMessageObj.items) {
            console.error("消息格式不正确:", getMessageObj);
            return;
        }

        const items = getMessageObj.items;

        // 更新 sensorList 的值
        this.setData({
            sensorList: this.data.sensorList.map(sensor => {
                let updatedSensor = {
                    ...sensor
                };

                // 根据参数名称更新对应的传感器数据
                switch (sensor.parameter) {
                    case "温度":
                        if (items.CurrentTemperature && typeof items.CurrentTemperature.value === "number") {
                            updatedSensor.value = items.CurrentTemperature.value;
                            updatedSensor.isPass = true; // 可以添加额外的逻辑来设置 isPass
                        }
                        break;
                    case "湿度":
                        if (items.CurrentHumidity && typeof items.CurrentHumidity.value === "number") {
                            updatedSensor.value = items.CurrentHumidity.value;
                            updatedSensor.isPass = true; // 可以添加额外的逻辑来设置 isPass
                        }
                        break;
                    case "光强":
                        if (items.LightValue && typeof items.LightValue.value === "number") {
                            updatedSensor.value = items.LightValue.value;
                            updatedSensor.isPass = true; // 可以添加额外的逻辑来设置 isPass
                        }
                        break;
                    case "接近感应":
                        if (items.DetectDistance && typeof items.DetectDistance.value === "number") {
                            updatedSensor.value = items.DetectDistance.value;
                            updatedSensor.isPass = true; // 可以添加额外的逻辑来设置 isPass
                        }
                        break;
                    default:
                        // 如果参数名称不匹配，则不进行更新
                        break;
                }
                return updatedSensor;
            })
        });
    },

    // 连接服务器按钮点击事件
    onClick_connect: function () {
        this.connectServer();
    },
    // 断开服务器按钮点击事件
    onClick_disconnect: function () {
        this.disconnectServer();
    },
    // 订阅按钮点击事件
    onClick_SubOne: function () {
        if (this.data.isMQTTSeverConnect === false) {
            wx.showToast({
                title: '请先连接MQTT服务器',
                icon: 'none',
                duration: 1000
            });
            return;
        }
        wx.setStorageSync('topicGet', this.data.topicGet);
        this.subscribeTopic(this.data.topicGet);
    },
    // 取消订阅按钮点击事件
    onClick_unSubOne: function () {
        this.unsubscribeTopic(this.data.topicGet);
    },
    // 发布主题按钮确认点击事件
    onClick_confirmPost: function () {
        var that = this;
        if (that.data.isMQTTSeverConnect === false) {
            wx.showToast({
                title: '请先连接MQTT服务器',
                icon: 'none',
                duration: 1000
            });
            return;
        }

        wx.setStorageSync('topicPost', that.data.topicPost);
        wx.showToast({
            title: '修改成功'
        });
        that.setData({
            isTopicModify: true
        });
        // this.publishTopic(that.data.topicGet, 'test i am from wechat msg');
        // console.log("isTopicModify:",that.data.isTopicModify);
    },
    // 修改主题按钮点击事件
    onClick_modifyPost: function () {
        var that = this;
        wx.showToast({
            title: '修改发布主题',
            icon: 'none',
            duration: 2000
        });
        that.setData({
            isTopicModify: false
        });
        // console.log("isTopicModify:",that.data.isTopicModify);
    },
    // led、蜂鸣器状态切换
    systemchange(e) {
        let that = this;
        if (that.data.isMQTTSeverConnect === false) {
            wx.showToast({
                title: '请先连接MQTT服务器',
                icon: 'none',
                duration: 1000
            });
            return;
        }
        let clickdata = e.target.dataset.param;
        let value;
        if (e.detail.value) {
            value = "on"
        } else {
            value = "off"
        }
        let msg = clickdata.label + value;
        console.log(msg);
        that.publishTopic(this.data.topicPost, msg);
    },
    // LED矩阵状态切换
    ledn_button_change(e) {
        let that = this;
        if (that.data.isMQTTSeverConnect === false) {
            wx.showToast({
                title: '请先连接MQTT服务器',
                icon: 'none',
                duration: 1000
            });
            return;
        }
        let value;
        that.setData({
            ledn_isOpen: e.detail.value
        });
        // console.log(that.data.ledn_isOpen)
        if (e.detail.value) {
            if (that.data.ledn_select_value === 0) {
                value = '0';
            } else if (that.data.ledn_select_value === 1) {
                value = '1';
            }else if (that.data.ledn_select_value === 2) {
                value = '2';
            }else if (that.data.ledn_select_value === 3) {
                value = '3';
            }else if (that.data.ledn_select_value === 4) {
                value = '4';
            }else if (that.data.ledn_select_value === 5) {
                value = '5';
            }else{
                return;
            }

        } else {
            value = "off"
        }
        let msg = 'ledn' + value;
        console.log(msg);
        that.publishTopic(this.data.topicPost, msg);
    },
    // LED矩阵状态切换
    ledn_select_change(e) {

        let that = this;
        that.setData({
            ledn_select_value: e.detail
        });

        // console.log(that.data.ledn_select_value);
        if (that.data.isMQTTSeverConnect === false) {
            wx.showToast({
                title: '请先连接MQTT服务器',
                icon: 'none',
                duration: 1000
            });
            return;
        }
        if (that.data.ledn_isOpen === false) {
            wx.showToast({
                title: '请先开启LED矩阵',
                icon: 'none',
                duration: 1000
            });
            return;
        }
        let value;
        if (that.data.ledn_select_value === 0) {
            value = '0';
        } else if (that.data.ledn_select_value === 1) {
            value = '1';
        }else if (that.data.ledn_select_value === 2) {
            value = '2';
        }else if (that.data.ledn_select_value === 3) {
            value = '3';
        }else if (that.data.ledn_select_value === 4) {
            value = '4';
        }else if (that.data.ledn_select_value === 5) {
            value = '5';
        }else if (that.data.ledn_select_value === 6) {
            value = '6';
        }else if (that.data.ledn_select_value === 7) {
            value = '7';
        }else{
            return;
        }
        let msg = 'ledn' + value;
        console.log(msg);
        that.publishTopic(this.data.topicPost, msg);
        console.log(this.data.topicPost);
    },

})