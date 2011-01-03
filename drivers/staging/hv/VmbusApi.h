


#ifndef _VMBUS_API_H_
#define _VMBUS_API_H_

#define MAX_PAGE_BUFFER_COUNT				16
#define MAX_MULTIPAGE_BUFFER_COUNT			32 

#pragma pack(push, 1)


struct hv_page_buffer {
	u32 Length;
	u32 Offset;
	u64 Pfn;
};


struct hv_multipage_buffer {
	
	u32 Length;
	u32 Offset;
	u64 PfnArray[MAX_MULTIPAGE_BUFFER_COUNT];
};


#define MAX_PAGE_BUFFER_PACKET		(0x18 +			\
					(sizeof(struct hv_page_buffer) * \
					 MAX_PAGE_BUFFER_COUNT))
#define MAX_MULTIPAGE_BUFFER_PACKET	(0x18 +			\
					 sizeof(struct hv_multipage_buffer))


#pragma pack(pop)

struct hv_driver;
struct hv_device;

struct hv_dev_port_info {
	u32 InterruptMask;
	u32 ReadIndex;
	u32 WriteIndex;
	u32 BytesAvailToRead;
	u32 BytesAvailToWrite;
};

struct hv_device_info {
	u32 ChannelId;
	u32 ChannelState;
	struct hv_guid ChannelType;
	struct hv_guid ChannelInstance;

	u32 MonitorId;
	u32 ServerMonitorPending;
	u32 ServerMonitorLatency;
	u32 ServerMonitorConnectionId;
	u32 ClientMonitorPending;
	u32 ClientMonitorLatency;
	u32 ClientMonitorConnectionId;

	struct hv_dev_port_info Inbound;
	struct hv_dev_port_info Outbound;
};

struct vmbus_channel_interface {
	int (*Open)(struct hv_device *Device, u32 SendBufferSize,
		    u32 RecvRingBufferSize, void *UserData, u32 UserDataLen,
		    void (*ChannelCallback)(void *context),
		    void *Context);
	void (*Close)(struct hv_device *device);
	int (*SendPacket)(struct hv_device *Device, const void *Buffer,
			  u32 BufferLen, u64 RequestId, u32 Type, u32 Flags);
	int (*SendPacketPageBuffer)(struct hv_device *dev,
				    struct hv_page_buffer PageBuffers[],
				    u32 PageCount, void *Buffer, u32 BufferLen,
				    u64 RequestId);
	int (*SendPacketMultiPageBuffer)(struct hv_device *device,
					 struct hv_multipage_buffer *mpb,
					 void *Buffer,
					 u32 BufferLen,
					 u64 RequestId);
	int (*RecvPacket)(struct hv_device *dev, void *buf, u32 buflen,
			  u32 *BufferActualLen, u64 *RequestId);
	int (*RecvPacketRaw)(struct hv_device *dev, void *buf, u32 buflen,
			     u32 *BufferActualLen, u64 *RequestId);
	int (*EstablishGpadl)(struct hv_device *dev, void *buf, u32 buflen,
			      u32 *GpadlHandle);
	int (*TeardownGpadl)(struct hv_device *device, u32 GpadlHandle);
	void (*GetInfo)(struct hv_device *dev, struct hv_device_info *devinfo);
};


struct hv_driver {
	const char *name;

	
	struct hv_guid deviceType;

	int (*OnDeviceAdd)(struct hv_device *device, void *data);
	int (*OnDeviceRemove)(struct hv_device *device);
	void (*OnCleanup)(struct hv_driver *driver);

	struct vmbus_channel_interface VmbusChannelInterface;
};


struct hv_device {
	
	struct hv_driver *Driver;

	char name[64];

	
	struct hv_guid deviceType;

	
	struct hv_guid deviceInstance;

	void *context;

	
	void *Extension;
};


struct vmbus_driver {
	
	
	struct hv_driver Base;

	
	struct hv_device * (*OnChildDeviceCreate)(struct hv_guid *DeviceType,
						struct hv_guid *DeviceInstance,
						void *Context);
	void (*OnChildDeviceDestroy)(struct hv_device *device);
	int (*OnChildDeviceAdd)(struct hv_device *RootDevice,
				struct hv_device *ChildDevice);
	void (*OnChildDeviceRemove)(struct hv_device *device);

	
	int (*OnIsr)(struct hv_driver *driver);
	void (*OnMsgDpc)(struct hv_driver *driver);
	void (*OnEventDpc)(struct hv_driver *driver);
	void (*GetChannelOffers)(void);

	void (*GetChannelInterface)(struct vmbus_channel_interface *i);
	void (*GetChannelInfo)(struct hv_device *dev,
			       struct hv_device_info *devinfo);
};

int VmbusInitialize(struct hv_driver *drv);

#endif 
