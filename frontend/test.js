// test.js
import { webSocket } from 'rxjs/webSocket';
import { timer, take, finalize, retryWhen, delay } from 'rxjs';
import { WebSocket } from 'ws';  // thu vien WebSocket cho Node.js

const WS_URL = 'ws://127.0.0.1:9001';
console.log(`Dang co gang ket noi toi ${WS_URL} bang RxJS...`);

// Ham lay thoi gian hien tai
function now() {
    return new Date().toLocaleTimeString();
}

// 1. Tao WebSocketSubject voi cau hinh chi tiet
function createWebSocket() {
    const subject = webSocket({
        url: WS_URL,
        WebSocketCtor: WebSocket,
        deserializer: (msg) => msg.data,

        openObserver: {
            next: () => {
                console.log(`[${now()}] Ket noi WebSocket thanh cong`);
            }
        },
        closeObserver: {
            next: (event) => {
                console.log(`[${now()}] Ket noi da dong. Ma: ${event.code}`);
            }
        },
        closingObserver: {
            next: () => {
                console.log(`[${now()}] Dang dong ket noi...`);
            }
        }
    });

    // 2. Dang ky nhan du lieu
    subject.pipe(
        retryWhen(errors => errors.pipe(
            delay(3000) // thu lai sau 3 giay neu mat ket noi
        ))
    ).subscribe({
        next: (msg) => {
            console.log(`[${now()}] Nhan du lieu: ${msg}`);
        },
        error: (err) => {
            console.error(`[${now()}] Loi ket noi:`, err);
        },
        complete: () => {
            console.log(`[${now()}] Luong du lieu hoan tat.`);
        }
    });

    // 3. Gui du lieu theo chu ky (5 lan moi giay)
    timer(0, 1000).pipe(
        take(5),
        finalize(() => {
            console.log(`[${now()}] Hoan tat chu ky gui, dong ket noi`);
            subject.complete();
        })
    ).subscribe(i => {
        const message = JSON.stringify({ command: "DATA", index: i + 1, time: now() });
        console.log(`[${now()}] Gui du lieu: ${message}`);
        subject.next(message);
    });
}

// Goi ham tao ket noi lan dau
createWebSocket();
