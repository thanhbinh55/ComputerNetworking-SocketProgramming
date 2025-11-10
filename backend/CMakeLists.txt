# Tên file: CMakeLists.txt

# Phiên bản CMake tối thiểu
cmake_minimum_required(VERSION 3.10)

# Tên dự án
project(RemoteControlServer CXX)

# Tên file nguồn
set(SOURCE_FILES main.cpp)

# Cài đặt thư viện
# Định nghĩa các gói cần tìm (Boost.Asio và Boost.Beast)
find_package(Boost REQUIRED COMPONENTS asio beast)

# Tạo tệp thực thi từ main.cpp
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Liên kết với các thư viện Boost đã tìm thấy
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::asio
    Boost::beast
)

# Cấu hình Vcpkg (Thêm dòng này nếu bạn không dùng Visual Studio)
# Cần chỉ ra Toolchain file của Vcpkg nếu bạn dùng g++/clang
# set(CMAKE_TOOLCHAIN_FILE "C:/Users/binh/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")