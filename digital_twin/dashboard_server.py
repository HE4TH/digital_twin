# -*- coding: utf-8 -*-
# Rescue Relay Console을 평문 HTTP로 서빙 (ws:// 사용, 인증서 불필요)
import os
import socket

from flask import Flask, jsonify, send_from_directory

app = Flask(__name__)
DIR = os.path.dirname(os.path.abspath(__file__))


def get_lan_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        s.close()


@app.route("/")
def index():
    return send_from_directory(DIR, "dashboard.html")


@app.route("/ip")
def ip():
    return jsonify({"ip": get_lan_ip(), "hostname": socket.gethostname()})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5050)
