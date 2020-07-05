#  High concurrency 
 c++ 高并发服务器项目
 
 ----------------------------windows中编译的注意事项-----------------------------
 vs 2015 更改调试环境的输出目录和中间配置目录：
 中间目录：$(SolutionDir)../../temp/$(Platform)/$(Configuration)/$(ProjectName)		//注意这里的../  表示上一级目录
 输出目录：$(SolutionDir)../../bin/$(Platform)/$(Configuration)


注意在配置中 链接器->输入 添加依赖项  ws2_32.lib



----------------------------Linux中编译的注意事项-----------------------------
在linux下编译文件：
g++ client.cpp -std=c++11 -pthread -o client
-std=c++11   是因为使用了vectoer 模板
-pthread     是因为使用了线程
