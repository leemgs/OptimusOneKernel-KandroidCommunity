

int cvm_oct_xmit(struct sk_buff *skb, struct net_device *dev);
int cvm_oct_xmit_pow(struct sk_buff *skb, struct net_device *dev);
int cvm_oct_transmit_qos(struct net_device *dev, void *work_queue_entry,
			 int do_free, int qos);
void cvm_oct_tx_shutdown(struct net_device *dev);


static inline void cvm_oct_free_tx_skbs(struct octeon_ethernet *priv,
					int skb_to_free,
					int qos, int take_lock)
{
	
	if (skb_to_free > 0) {
		if (take_lock)
			spin_lock(&priv->tx_free_list[qos].lock);
		while (skb_to_free > 0) {
			dev_kfree_skb(__skb_dequeue(&priv->tx_free_list[qos]));
			skb_to_free--;
		}
		if (take_lock)
			spin_unlock(&priv->tx_free_list[qos].lock);
	}
}
