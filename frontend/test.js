// test.js
import { webSocket } from 'rxjs/webSocket';
import { timer, take, finalize } from 'rxjs'; 

// Import thư viện WebSocket cho Node.js (ws)
import { WebSocket } from 'ws'; 

const WS_URL = 'ws://127.0.0.1:8080';
console.log(`Đang cố gắng kết nối tới ${WS_URL} bằng RxJS...`);

// 1. Tạo WebSocketSubject
// *** Tối quan trọng: Cấu hình WebSocketCtor để dùng thư viện 'ws' ***
const subject = webSocket({
    url: WS_URL,
    // Cung cấp Constructor của WebSocket từ thư viện 'ws'
    WebSocketCtor: WebSocket,
    deserializer: (messageEvent) => {
        return messageEvent.data;
    },

    // Theo dõi kết nối thành công
    openObserver: {
        next: () => {
            console.log('\n✅ RxJS: Kết nối WebSocket đã thiết lập thành công!');
        }
    },
    // Theo dõi đóng kết nối
    closeObserver: {
        next: (event) => {
            console.log(`\n❌ RxJS: Kết nối đã đóng. Code: ${event.code}`);
        }
    },
    // Theo dõi lỗi
    closingObserver: {
        next: () => {
             console.log('...Đang trong quá trình đóng kết nối...');
        }
    }
});

// 2. Nhận dữ liệu (Subscribe)
subject.subscribe({
    next: (msg) => {
        console.log('📬 RxJS Nhận Dữ Liệu:', msg);
    },
    error: (err) => {
        console.error('⚠️ Lỗi Kết Nối RxJS:', err);
    },
    complete: () => {
        console.log('🛑 RxJS: Luồng dữ liệu hoàn tất.');
    }
});


// 3. Gửi dữ liệu theo chu kỳ (Mô phỏng giao tiếp)
// Gửi 5 tin nhắn mỗi giây, sau đó đóng kết nối
timer(0, 1000).pipe(
    take(5), // Chỉ gửi 5 lần
    // finalize được gọi khi luồng hoàn tất
    finalize(() => {
        console.log('--- Hoàn tất chu kỳ gửi tin, đóng kết nối ---');
        subject.complete(); // Đóng kết nối WebSocket
    })
).subscribe(i => {
    const message = `{"command": "DATA", "index": ${i + 1}}`;
    console.log(`➡️ RxJS Gửi Dữ Liệu: ${message}`);
    
    // Gửi tin nhắn
    subject.next(message);
});
