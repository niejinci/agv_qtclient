/**
 * @file point_cloud_compression.h
 * @brief 提供用于压缩和解压缩三维点云数据的方法。
 *
 * 本文件包含用于高效压缩点云数据的工具，通过将三维坐标量化为8位值，
 * 在保持可视化和导航精度的同时显著节省空间。
 */

 #pragma once

 #include <cstdint>
 #include <vector>
 #include <limits>
 #include <chrono>
 
 namespace agv {
 namespace compression {
 
 /**
  * @brief 表示未压缩的三维点的结构体。
  */
 struct Point3D {
     float x;
     float y;
     float z;
 };
 
 /**
  * @brief 表示使用8位坐标压缩的三维点的结构体。
  */
 struct CompressedPoint3D {
     uint8_t x;
     uint8_t y;
     uint8_t z;
 };
 
 /**
  * @brief 压缩和解压缩点云所需的元数据。
  */
 struct CompressionBounds {
     float x_min;
     float x_max;
     float y_min;
     float y_max;
     float z_min;
     float z_max;
 };
 
 /**
  * @brief 获取当前时间（以毫秒为单位）。
  *
  * @return 自纪元以来的当前时间（以毫秒为单位）。
  */
 static double get_time_in_ms() {
     auto now = std::chrono::system_clock::now();
     auto duration = now.time_since_epoch();
     return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
 }
 
 /**
  * @brief 将压缩边界和压缩点序列化为字节数组。
  *
  * @param bounds 要序列化的压缩边界。
  * @param compressed_points 要序列化的压缩点。
  * @return 包含序列化数据的字节数组。
  */
 static std::vector<uint8_t> serialize_to_byte_array(
     const CompressionBounds& bounds,
     const std::vector<CompressedPoint3D>& compressed_points) {
     std::vector<uint8_t> binary_data;
     binary_data.reserve(sizeof(CompressionBounds) + compressed_points.size() * sizeof(CompressedPoint3D));
 
     // 序列化边界
     const uint8_t* byte_bounds = reinterpret_cast<const uint8_t*>(&bounds);
     binary_data.insert(binary_data.end(), byte_bounds, byte_bounds + sizeof(CompressionBounds));
 
     // 序列化压缩点
     for (const auto& point : compressed_points) {
         const uint8_t* byte = reinterpret_cast<const uint8_t*>(&point);
         binary_data.insert(binary_data.end(), byte, byte + sizeof(CompressedPoint3D));
     }
 
     return binary_data;
 }
 
 /**
  * @brief 将字节数组反序列化为压缩边界和压缩点。
  *
  * @tparam T 输入二进制数据的类型（std::vector<uint8_t> 或 std::string）。
  * @param binary_data 包含序列化数据的字节数组。
  * @param bounds 反序列化的压缩边界。
  * @param compressed_points 反序列化的压缩点。
  */
 template <typename T>
 static void deserialize_from_byte_array(
     const T& binary_data,
     CompressionBounds& bounds,
     std::vector<CompressedPoint3D>& compressed_points) {
     // 反序列化边界
     bounds = *reinterpret_cast<const CompressionBounds*>(binary_data.data());
 
     // 反序列化压缩点
     size_t base_offset = sizeof(CompressionBounds);
     size_t step = sizeof(CompressedPoint3D);
     size_t num_points = (binary_data.size() - base_offset) / step;
 
     compressed_points.clear();
     compressed_points.reserve(num_points);
 
     for (size_t i = base_offset; i < binary_data.size(); i += step) {
         CompressedPoint3D point = *reinterpret_cast<const CompressedPoint3D*>(binary_data.data() + i);
         compressed_points.push_back(point);
     }
 }
 
 /**
  * @brief 用于压缩和解压缩三维点云数据的类。
  */
 class PointCloudCompressor {
 public:
     /**
      * @brief 计算一组三维点的边界信息。
      *
      * @param points 要分析的输入点。
      * @return 计算出的边界信息。
      */
     static CompressionBounds calculate_bounds(const std::vector<Point3D>& points) {
         CompressionBounds bounds;
 
         // 初始化为最大/最小可能值
         bounds.x_min = bounds.y_min = bounds.z_min = std::numeric_limits<float>::max();
         bounds.x_max = bounds.y_max = bounds.z_max = std::numeric_limits<float>::lowest();
 
         // 找到每个维度的最小值和最大值
         for (const auto& point : points) {
             bounds.x_min = std::min(bounds.x_min, point.x);
             bounds.x_max = std::max(bounds.x_max, point.x);
             bounds.y_min = std::min(bounds.y_min, point.y);
             bounds.y_max = std::max(bounds.y_max, point.y);
             bounds.z_min = std::min(bounds.z_min, point.z);
             bounds.z_max = std::max(bounds.z_max, point.z);
         }
 
         return bounds;
     }
 
     /**
      * @brief 压缩单个三维点。
      *
      * @param point 要压缩的输入点。
      * @param bounds 用于压缩的边界信息。
      * @return 压缩后的点。
      */
     static CompressedPoint3D compress_point(
         const Point3D& point,
         const CompressionBounds& bounds) {
         CompressedPoint3D compressed;
 
         // 计算坐标范围
         float x_range = bounds.x_max - bounds.x_min;
         float y_range = bounds.y_max - bounds.y_min;
         float z_range = bounds.z_max - bounds.z_min;
 
         // 避免除以零
         x_range = (x_range == 0) ? 1.0f : x_range;
         y_range = (y_range == 0) ? 1.0f : y_range;
         z_range = (z_range == 0) ? 1.0f : z_range;
 
         // 执行量化
         float normalized_x = (point.x - bounds.x_min) / x_range;
         float normalized_y = (point.y - bounds.y_min) / y_range;
         float normalized_z = (point.z - bounds.z_min) / z_range;
 
         // 将 [0,1] 范围映射到 [0,255]
         compressed.x = static_cast<uint8_t>(normalized_x * 255.0f + 0.5f);
         compressed.y = static_cast<uint8_t>(normalized_y * 255.0f + 0.5f);
         compressed.z = static_cast<uint8_t>(normalized_z * 255.0f + 0.5f);
 
         return compressed;
     }
 
     /**
      * @brief 解压缩单个压缩的三维点。
      *
      * @param compressed_point 输入的压缩点。
      * @param bounds 压缩期间使用的边界信息。
      * @return 解压缩后的三维点。
      */
     static Point3D decompress_point(
         const CompressedPoint3D& compressed_point,
         const CompressionBounds& bounds) {
         Point3D decompressed;
 
         // 计算坐标范围
         float x_range = bounds.x_max - bounds.x_min;
         float y_range = bounds.y_max - bounds.y_min;
         float z_range = bounds.z_max - bounds.z_min;
 
         // 从 [0,255] 反量化到 [0,1]
         float normalized_x = compressed_point.x / 255.0f;
         float normalized_y = compressed_point.y / 255.0f;
         float normalized_z = compressed_point.z / 255.0f;
 
         // 将归一化值转换回原始范围
         decompressed.x = bounds.x_min + normalized_x * x_range;
         decompressed.y = bounds.y_min + normalized_y * y_range;
         decompressed.z = bounds.z_min + normalized_z * z_range;
 
         return decompressed;
     }
 
     /**
      * @brief 压缩一组三维点。
      * 
      * @param points 要压缩的输入点。
      * @return 压缩后的点的向量。
      */
     static std::vector<CompressedPoint3D> compress_point_cloud(const std::vector<Point3D>& points) {
         // 计算压缩边界
         CompressionBounds bounds = calculate_bounds(points);
 
         // 压缩所有点
         std::vector<CompressedPoint3D> compressed_points;
         compressed_points.reserve(points.size());
 
         for (const auto& point : points) {
             compressed_points.push_back(compress_point(point, bounds));
         }
 
         return compressed_points;
     }
 
     /**
      * @brief 解压缩一组压缩的三维点。
      * 
      * @param compressed_points 输入的压缩点。
      * @param bounds 压缩期间使用的边界信息。
      * @return 解压缩后的三维点的向量。
      */
     static std::vector<Point3D> decompress_point_cloud(
         const std::vector<CompressedPoint3D>& compressed_points,
         const CompressionBounds& bounds) {
         std::vector<Point3D> decompressed_points;
         decompressed_points.reserve(compressed_points.size());
 
         for (const auto& compressed_point : compressed_points) {
             decompressed_points.push_back(decompress_point(compressed_point, bounds));
         }
 
         return decompressed_points;
     }
 
 private:
     // 禁止创建此对象的实例
     PointCloudCompressor() = delete;
 };
 
 }  // namespace compression
 }  // namespace agv