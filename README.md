[toc]
***

## ros2 主题梳理


| 类型 | 主题               | 频率(hz)                                  | 说明                          | api                     | 定时器                          | 定时器频率 |
|------|--------------------|------------------------------------------|-------------------------------|-------------------------|---------------------------------|------------|
| 订阅 | agv_di_topic       | 按需发布                                 | plc di                        | 无                      |                                 |            |
|      | agv_state_topic    | 任务状态变化时发布，无变化时1hz          |  任务/动作状态信息              |                         |                                 |            |
|      | filte_scan         | 30                                     | 点云数据                      | get_point_cloud         | get_point_cloud_timer_          | 100ms      |
|      | camera/depth/points| 10hz                                     | 3d摄像头点云数据               | get_3dcamera_pointcloud | get_3dcamera_pointcloud_timer_  | 200ms      |
|      | locationInfo       | 30                                      | 小车位置信息                  | get_agv_position        | get_agv_position_timer_         | 100ms      |
|      | mcu_to_pc          | 50                                     | 点击和电量信息                | get_mcu2pc              |                                 |            |
|      | sys_info           | 1hz                                      | 系统信息                      | get_sysinfo             |                                 |            |
|      | qr_pos_data        | 扫到地码时才发布                          |                               | get_qr_camera_data      |                                 |            |
|      | qr_rack_data       | 扫到地码时才发布                          | 扫描相机扫到码时发布的数据      | get_qr_camera_data      |                                 |            |
|      | scan2pointcloud    | 30                                      | 障碍物点云                    | get_scan2pointcloud     | get_scan2pointcloud_timer_      | 100ms      |
|      | obst_polygon       | 30                                      | 障碍轮廓点云                  | get_obst_polygon        | get_obst_polygon_timer_         | 100ms      |
|      | obst_pcl           | 30                                      | 障碍物点云                    | get_obst_pcl            | get_obst_pcl_timer_             | 100ms      |
| 发布 | agv_order_topic    | 按需发布                                 | 任务主题                      | 无                      |                                 |            |
|      | agv_instant_topic  | 按需发布                                 | 即时动作主题                  | 无                      |                                 |            |
|      | agv_do_topic       | 按需发布                                 | plc do                        | 无                      |                                 |            |


## ros2 主题命令

### 查看主题发布的频率

ros2 topic hz /filte_scan

ros2 topic hz /locationInfo

ros2 topic hz /locationInfo

### 查看主题的内容

ros2 topic echo /scan2pointcloud --csv

ros2 topic echo /obst_polygon

ros2 topic echo /agv_instant_topic

ros2 topic echo /model_polygon

ros2 topic echo /agv_state_topic
