
    最低支持内核版本: GNU/Linux 2.6.32

    root 安装 root 运行 (sh ./install 默认完成后自动运行 daemon)
    
    log 可查看运行实时信息 tailf /usr/local/kv_edr_client/log/edr_client_log
    安装目录:/usr/local/kv_edr_client/  
    
    配置文件:/usr/local/kv_edr_client/etc/edr_config.xml	
    
    修改地址需编辑 "center_agent" 与 "sysinfo" 两个模块的参数
    
    脚本路径 /usr/local/kv_edr_client/script
    
    ./kill_kv_edr_client.sh 中止运行 daemon (root 用户开机自启)
    
    ./run_kv_edr_client.sh 启动服务
    
    ./uninstall.sh 终止并卸载程序
