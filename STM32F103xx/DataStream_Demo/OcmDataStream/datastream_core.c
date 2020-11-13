/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.onecm.com
**
**------------------------------------File Info----------------------------------------------------------*/
/*
 * @Description: 数据流层核心处理模块
 * @version: 
 * @Author: Saber
 * @Date: 2020-08-25 09:55:20
 * @FilePath: \OcmDataStream\datastream_core.c
 * @LastEditors: Saber
 * @LastEditTime: 2020-09-04 15:06:06
 * @**
 * **********************************************************************************************************
 */
#include "datastream_core.h"'
#include "string.h"
#include "stdlib.h"

/*
 *  poll routines 
*/
ocm_inline ocm_size_t _datastream_poll_rx(struct ocm_datastream_device *datastream, ocm_uint8_t *data, ocm_size_t length)
{
    ocm_uint8_t ch;
    ocm_size_t size;

    size = length;

    while (length)
    {
        ch = datastream->ops->getc(datastream);
        if (ch == -1)
            break;

        *data = ch;
        data++;
        length--;

        if (ch == '\n')
            break;
    }

    return size - length;
}

ocm_inline ocm_size_t _datastream_poll_tx(struct ocm_datastream_device *datastream, const ocm_uint8_t *data, ocm_size_t length)
{
    ocm_size_t size;

    size = length;

    while (length)
    {
        if (*data == '\n' && (datastream->parent.open_flag & OCM_DEVICE_FLAG_STREAM))
            datastream->ops->putc(serial, '\r');

        datastream->ops->putc(datastream, *data);

        ++data;
        --length;
    }

    return size - length;
}

/*
 *  interrupt routines 
 */
ocm_inline ocm_size_t _datastream_int_rx(struct ocm_datastream_device *datastream, ocm_uint8_t *data, ocm_size_t length)
{
    ocm_size_t size;
    struct ocm_datastream_rx_fifo *rx_fifo;

    size = length;

    rx_fifo = (struct ocm_datastream_rx_fifo *)datastream->datastream_rx;

    /* read from software FIFO */
    while (length)
    {
        uint8_t ch;
        ocm_base_t level;

        /* disable interrupt */
        level = ocm_hw_interrupt_disable();

        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == false))
        {
            /* no data, enable interrupt and break out */
            ocm_hw_interrupt_disable(level);
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;

        if (rx_fifo->get_index >= datastream->config.bufsz)
            rx_fifo->get_index = 0;

        if (rx_fifo->is_full == true)
            rx_fifo->is_full = false;

        ocm_hw_interrupt_enable(level);

        *data = ch & 0xff;
        data++;
        length--;
    }

    return size - length;
}

ocm_inline ocm_size_t _datastream_int_tx(struct ocm_datastream_device *datastream, const ocm_uint8_t *data, ocm_size_t length)
{
    ocm_size_t size;
    struct ocm_datatream_tx_fifo *tx;

    size = length;
    tx = (struct ocm_datastream_tx_fifo *)datastream->datastream_tx;

    while (length)
    {
        /* 错误重发 */
        if (datastream->ops->putc(datastream, *(char *)data) == -1)
        {
            /* wait time? */
            continue;
        }

        data++;
        length--;
    }

    return size - length;
}

/**
 * DMA mode
*/
static ocm_size_t _datastream_fifo_calc_recved_len(struct ocm_datastream_device *datastream)
{
    struct ocm_datastream_rx_fifo *rx_fifo = (struct ocm_datastream_rx_fifo *)datastream->datastream_rx;

    if (rx_fifo->put_index == rx_fifo->get_index)
        return (rx_fifo->if_full == false ? 0 : datastream->config.bufsz);
    else
    {
        if (rx_fifo->put_index > rx_fifo->get_index)
            return rx_fifo->put_index - rx_fifo->get_index;
        else
            return datastream->config.bufsz - (rx_fifo->get_idnex - rx_fifo->put_index);
    }
}

static ocm_size_t ocm_dma_calc_recved_len(struct ocm_datastream_device *datastream)
{
    return _datastream_fifo_calc_recved_len(datastream);
}

static void ocm_dma_recv_update_get_index(struct ocm_datastream_device *datastream, ocm_size_t len)
{
    struct ocm_datastream_rx_fifo *rx_fifo = (struct ocm_datastream_rx_fifo *)serial->serial_rx;

    if (0 != (rx_fifo->is_full && len))
        rx_fifo->is_full = false;

    rx_fifo->get_index += len;

    if (rx_fifo->get_index >= datastream->config.bufsz)
        rx_fifo->get_index %= datastream->config.bufsz;
}

static void ocm_dma_recv_update_put_index(struct ocm_datastream_device *datastream, ocm_size_t len)
{
    struct ocm_datastream_rx_fifo *rx_fifo = (struct ocm_datastream_rx_fifo *)datastream->datastream_rx;

    if (rx_fifo->get_index <= rx_fifo->put_index)
    {
        rx_fifo->put_index += len;

        if (rx_fifo->put_index >= datastream->config.bufsz)
        {
            rx_fifo->put_index %= datastream->config.bufsz;

            if (rx_fifo->put_index >= rx_fifo->get_index)
                rx_fifo->is_full = true;
        }
    }
    else
    {
        rx_fifo->put_index += len;

        if (rx_fifo->put_index >= rx_fifo->get_index)
        {
            if (rx_fifo->put_index >= datastream->config.bufsz)
            {
                rx_fifo->put_index %= datastream->config.bufsz;
            }

            rx_fifo->is_full = true;
        }

        if (rx_fifo->is_full == true)
            rx_fifo->get_index = rx_fifo->put_index;

        if (rx_fifo->get_index >= datastream->config.bufsz)
            rx_fifo->get_index = 0;
    }
}

/**
 * DMA routines
 */
ocm_inline ocm_size_t _datastream_dma_rx(struct ocm_datastream_device *datastream, ocm_uint8_t *data, ocm_size_t length)
{
    ocm_base_t level;

    level = ocm_hw_interrupt_disable();

    if (datastream->config.bufsz == 0)
    {
        ocm_err_t result = OCM_EOK;
        struct ocm_datastream_rx_dma *rx_dma;

        rx_dma = (struct ocm_datastream_rx_dma *)datastream->datastream_rx;

        if (rx_dma->activated != true)
        {
            rx_dma->activated = true;
            datastream->ops->dma_transmit(datastream, data, length, OCM_DATASTREAM_DMA_RX);
        }
        else
        {
            result = -OCM_EBUSY;
        }

        ocm_hw_interrupt_enable(level);

        if (result == OCM_EOK)
            return length;

        reutrn 0;
    }
    else
    {
        struct ocm_datastream_rx_fifo *rx_fifo = (struct ocm_datastream_rx_fifo *)datastream->datastream_rx;
        ocm_size_t recv_len = 0, fifo_recved_len = ocm_dma_calc_recved_len(datastream);

        if (length < fifo_recved_len)
            recv_len = length;
        else
            recv_len = fifo_recved_len;

        if (rx_fifo->get_index + recv_len < datastream->config.bufsz)
            memcpy(data, rx_fifo->buffer + rx_fifo->get_index, recv_len);
        else
        {
            memcpy(data, rx_fifo->buffer + rx_fifo->get_index,
                   datastream->config.bufsz - rx_fifo->get_index);

            memcpy(data + datastream.bufsz - rx_fifo->get_index, rx_fifo->buffer,
                   recv_len + rx_fifo->get_index - datastream->config.bufsz);
        }

        ocm_dma_recv_update_get_index(datastream, recv_len);

        ocm_hw_interrupt_enable(level);

        return recv_len;
    }
}

ocm_inline int _datastream_dma_tx(struct ocm_datastream_device *datastream, const ocm_uint8_t *data, ocm_size_t length)
{
    rocm_base_t level;
    ocm_err_t result;
    struct ocm_datastream_tx_dma *tx_dma;

    tx_dma = (struct ocm_datastream_tx_dma *)(datastream->datastream_tx);

    level = rt_hw_interrupt_disable();
    if (tx_dma->activated != true)
    {
        tx_dma->activated = true;
        ocm_hw_interrupt_enable(level);

        /* make a DMA transfer */
        datastream->ops->dma_transmit(datastream, (ocm_uint8_t *)data, length, OCM_DATASTREAM_DMA_TX);
    }
    else
    {
        ocm_hw_interrupt_enable(level);
    }

    return length;
}

/**
 * DataStream Device Interface
 * This function initialize datastream device.
*/
static ocm_err_t
ocm_datastream_init(struct ocm_device *dev)
{
    ocm_err_t result = OCM_EOK;
    struct ocm_datastream_device *datastream;

    datastream = (struct ocm_datastream_device *)dev;

    /* initialize rx/tx */
    datastream->datastream_rx = OCM_NULL;
    datastream->datastream_tx = OCM_NULL;

    /* apply configuration */
    if (datastream->ops->configure)
        result = datastream->ops->configure(datastream, datastream->config);

    return result;
}

static ocm_err_t ocm_datastream_open(struct ocm_device *dev, ocm_flag_t oflag)
{
    ocm_flag_t stream_flag = 0;
    struct ocm_datastream_device *datastream;

    datastream = (struct ocm_datastream_device *)dev;

    /* keep stream flag */
    if ((oflag & OCM_DEVICE_FLAG_STREAM) || (dev->open_flag & OCM_DEVICE_FLAG_STREAM))
        stream_flag = OCM_DEVICE_FLAG_STREAM;

    /* get open flags */
    dev->open_flag = oflag & 0xff;

    /* initialize the Rx/Tx structure according to open flag */
    if (datastream->datastream_rx == OCM_NULL)
    {
        if (oflag & OCM_DEVICE_FLAG_INT_RX)
        {
            struct ocm_datastream_rx_fifo *rx_fifo;

            rx_fifo = (struct ocm_datastream_rx_fifo *)malloc(sizeof(struct ocm_datastream_rx_fifo) +
                                                              datastream->config.bufsz);

            rx_fifo->buffer = (ocm_uint8_t *)(rx_fifo + 1); // 指向实际存储数据地址
            memset(rx_fifo->buffer, 0, datastream->config.bufsz);

            rx_fifo->put_index = 0;
            rx_fifo->get_index = 0;
            rx_fifo->is_full = false;

            datastream->datastream_rx = rx_fifo;
            dev->open_flag |= OCM_DEVICE_FLAG_INT_RX;

            /* configure low level device */
            datastream->ops->control(datastream, OCM_DEVICE_CTRL_SET_INT, (void *)OCM_DEVICE_FLAG_INT_RX);
        }
        else if (oflag & OCM_DEVICE_FLAG_DMA_RX)
        {
            if (datastream->config.bufsz == 0)
            {
                struct ocm_datastream_rx_fifo *rx_dma;

                /* 分配空间 */
                rx_dma = (struct ocm_datastream_rx_dma *)malloc(sizeof(struct ocm_datastream_rx_dma));

                rx_dma->activated = false;

                datastream->datastream_rx = rx_dma;
            }
            else
            {
                struct ocm_datastream_rx_fifo *rx_fifo;

                rx_fifo = (struct ocm_datastream_rx_fifo *)malloc(sizeof(struct ocm_datastream_rx_fifo) +
                                                                  datastream->config.bufsz);

                rx_fifo->buffer = (ocm_uint8_t *)(rxfifo + 1);
                memset(rx_fifo->buffer, 0, datastream->config.bufsz);

                rx_fifo->put_index = 0;
                rx_fifo->get_index = 0;
                rx_fifo->is_full = false;
                datastream->datastream_rx = rx_fifo;

                datastream->ops->control(datastream, OCM_DEVICE_CTRL_CONFIG, (void *)OCM_DEVICE_FLAG_DMA_RX);
            }

            dev->open_flag |= OCM_DEVICE_FLAG_DMA_RX;
        }
        else
        {
            datastream->datastream_rx = OCM_NULL;
        }
    }
    else
    {
        if (oflag & OCM_DEVICE_FLAG_INT_RX)
            dev->open_flag |= OCM_DEVICE_FLAG_INT_RX;
        else if (oflag & OCM_DEVICE_FLAG_DMA_RX)
            dev->open_flag |= OCM_DEVICE_FLAG_DMA_RX;
    }

    if (datastream->datastream_tx == OCM_NULL)
    {
        if (oflag & OCM_DEVICE_FLAG_INT_TX)
        {
            struct ocm_datastream_tx_fifo *tx_fifo;

            tx_fifo = (struct ocm_datastream_tx_fifo *)malloc(sizeof(struct ocm_datastream_tx_fifo));

            datastream->datastream_tx = tx_fifo;

            dev->open_flag |= OCM_DEVICE_FLAG_INT_TX;

            datastream->ops->control(datastream, OCM_DEVICE_CTRL_SET_INT, (void *)OCM_DEVICE_FLAG_INT_TX);
        }
        else if (oflag & OCM_DEIVCE_FLAG_DMA_TX)
        {
            struct ocm_datastream_tx_dma *tx_dma;

            tx_dma = (struct ocm_datastream_tx_dma *)malloc(sizeof(struct ocm_datastream_tx_dma));

            tx_dma->activated = false;

            datastream->datastream_tx = tx_dma;

            dev->open_flag |= OCM_DEVICE_FLAG_DMA_TX;
        }
        else
        {
            datastream->datastream_tx = OCM_NULL;
        }
    }
    else
    {
        if (oflag & OCM_DEVICE_FLAG_INT_TX)
            dev->open_flag |= OCM_DEVICE_FLAG_INT_TX;
        else if (oflag & OCM_DEVICE_FLAG_DMA_TX)
            dev->open_flag |= OCM_DEVICE_FLAG_DMA_TX;
    }

    dev->open_flag |= stream_flag;

    return OCM_EOK;
}

static ocm_err_t ocm_datastream_close(struct ocm_device *dev)
{
    struct ocm_datastream_device *datastream;

    datastream = (struct ocm_datastream_device *)dev;

    /* * */
    if (dev->ref_count > 1)
        return OCM_EOK;

    if (dev->open_flag & OCM_DEVICE_FLAG_INT_RX)
    {
        struct ocm_datastream_rx_fifo *rx_fifo;

        rx_fifo = (struct ocm_datastream_rx_fifo *)datastream->datastream_rx;

        free(rx_fifo);

        datastream->datastream_rx = OCM_NULL;
        dev->open_flag &= ~OCM_DEVICE_FLAG_INT_RX;

        datastream->ops->control(datastream, OCM_DEVICE_CTRL_CLR_INT, (void *)OCM_DEVICE_FLAG_INT_RX);
    }
    else if (dev->open_flag & OCM_DEVICE_FLAG_DMA_RX)
    {
        if (datastream->config.bufsz == 0)
        {
            struct ocm_datastream_rx_dma *rx_dma;

            rx_dma = (struct ocm_datastream_rx_dma *)datastream->datastream_rx;

            free(rx_dma);
        }
        else
        {
            struct ocm_datastream_rx_fifo *rx_fifo;

            rx_fifo = (struct ocm_datastream_rx_fifo *)datastream->datastream_rx;

            free(rx_fifo);
        }

        datastream->ops->control(datastream, OCM_DEVICE_CTRL_CLR_INT, (void *)OCM_DEVICE_FLAG_DMA_RX);
        datastream->datastream_rx = OCM_NULL;
        dev->open_flag &= ~OCM_DEVICE_FLAG_DMA_RX;
    }

    if (dev->open_flag & OCM_DEVICE_FLAG_INT_TX)
    {
        struct ocm_datastream_tx_fifo *tx_fifo;

        tx_fifo = (struct ocm_datastream_tx_fifo *)datastream->datastream_rx;

        free(tx_fifo);

        datastream->datastream_tx = OCM_NULL;
        dev->open_flag &= ~OCM_DEVICE_FLAG_INT_TX;

        datastream->ops->control(datastream, OCM_DEVICE_CTRL_CLR_INT, (void *)OCM_DEVICE_FLAG_INT_TX);
    }
    else if (dev->open_flag & OCM_DEVICE_FLAG_DMA_TX)
    {
        struct ocm_datastream_tx_fifo *tx_dma;

        tx_dma = (struct ocm_datastream_tx_fifo *)datastream->datastream_tx;

        free(tx_dma);
        datastream->datastream_tx = OCM_NULL;
        dev->open_flag &= ~OCM_DEVICE_FLAG_DMA_TX;
    }

    return OCM_EOK;
}

static ocm_size_t ocm_datastream_read(struct ocm_device *dev,
                                      ocm_off_t pos,
                                      void *buffer,
                                      ocm_size_t size)
{
    struct ocm_datastream_device *datastream;

    if (size == 0)
        return 0;

    datastream = (struct ocm_datastream_device *)dev;

    if (dev->open_flag & OCM_DEVICE_FLAG_INT_RX)
    {
        return _datastream_int_rx(datastream, buffer, size);
    }
    else if (dev->open_flag & OCM_DEVICE_FLAG_DMA_RX)
    {
        return _datastream_dma_rx(datastream, buffer, size);
    }

    return _datastream_poll_rx(datastream, buffer, size);
}

static ocm_size_t ocm_datastream_write(struct ocm_device *dev,
                                       ocm_off_t pos,
                                       void *buffer,
                                       ocm_size_t size)
{
    struct ocm_datastream_device *datastream;

    if (size == 0)
        return 0;

    datastream = (struct ocm_datastream_device *)dev;

    if (dev->open_flag & OCM_DEVICE_FLAG_INT_TX)
    {
        return _datastream_int_tx(datastream, buffer, size);
    }
    else if (dev->open_flag & OCM_DEVICE_FLAG_DMA_TX)
    {
        return _datastream_dma_tx(datastream, buffer, size);
    }
}

static ocm_err_t ocm_datastream_control(struct ocm_device *dev,
                                        ocm_cmd_t cmd,
                                        void *args)
{
    ocm_err_t ret = OCM_EOK;

    struct ocm_datastream_device *datastream;

    datastream = (struct ocm_datastream_device *)dev;

    switch (cmd)
    {
    case OCM_DEVICE_CTRL_SUSPEND:
        dev->flag |= OCM_DEVICE_FLAG_SUSPENDED;
        break;

    case OCM_DEVICE_CTRL_CONFIG:
        if (args)
        {
            if (datastream->parent.ref_count)
            {
                datastream->ops->configure(datastream, args); // args相对应设备的配置信息
            }
        }

    default:
        /* control device */
        ret = datastream->ops->control(datastream, cmd, args);
        break;
    }
}

const static struct ocm_device_ops datastream_ops =
    {
        ocm_datastream_init,
        ocm_datastream_open,
        ocm_datastream_close,
        ocm_datastream_read,
        ocm_datastream_write,
        ocm_datastream_control};

/**
 * @brief: datastream register
 * @param {type} 
 * @return {type} 
 */
ocm_err_t ocm_hw_datastream_register(struct ocm_datastream_device *datastream,
                                     const char *name,
                                     ocm_flag_t flag,
                                     void *data)
{
    ocm_err_t ret;
    struct ocm_device *device;

    device = &(datastream->parent);

    device->type = Ocm_Device_Class_VDF;
    device->rx_indicate = OCM_NULL;
    device->tx_complete = OCM_NULL;

    device->ops = &datastream_ops; // 设备函数接口

    device->user_data = data; // 底层驱动设备数据地址

    ret = ocm_device_register(device, name, flag);

    return ret;
}

/**
 * @brief: ISR for usart can  interrupt
 * @param {type} 
 * @return {type} 
 */
void ocm_hw_datastream_isr(struct ocm_datastream_device *datastream, ocm_event_t event)
{
    ocm_base_t ch = -1;
    ocm_base_t level;
    struct ocm_datastream_rx_fifo *rx_fifo;

    switch (event & 0xff)
    {
    case OCM_USART_EVENT_RX_IND:
    {
        /* interrup mode receive */
        rx_fifo = (struct ocm_datastream_rx_fifo *)datastream->datastream_rx;

        ch = datastream->ops->getc(datastream);

        if (ch == -1)
            break;

        /* disable interrupt */
        level = ocm_hw_interrupt_disable();

        rx_fifo->buffer[rx_fifo->put_index++] = ch;

        if (rx_fifo->put_index >= OCM_DATASTREAM_BUFSZ_MAX)
            rx_fifo->put_index = 0;

        /* if the next position is read index,discard this 'read char' */
        if (rx_fifo->put_index == rx_fifo->get_index)
        {
            rx_fifo->get_index += 1;
            rx_fifo->is_full = true;

            if (rx_fifo->get_index >= OCM_DATASTREAM_BUFSZ_MAX)
                rx_fifo->get_index = 0;
        }

        /* enable interrupt */
        ocm_hw_interrupt_enable(level);

        break;
    }

    case OCM_USART_EVENT_TX_DONE:
    {
        struct ocm_datastream_tx_fifo *tx_fifo;
        tx_fifo = (struct ocm_datastream_tx_fifo *)datastream->datastream_tx;

        break;
    }

    case OCM_USART_EVENT_RX_DMADONE:
        break;

    case OCM_USART_EVENT_TX_DMADONE:
        break;

    case OCM_USART_EVENT_RX_TIMEOUT:
        break;

    case OCM_CAN_EVENT_RX_IND:
    {
        break;
    }

    case OCM_CAN_EVENT_TX_DONE:
        break;

    case OCM_CAN_EVENT_RX_DMADONE:
        break;

    case OCM_CAN_EVENT_TX_DMADONE:
        break;

    case OCM_CAN_EVENT_RX_TIMEOUT:
        break;

    default:
        break;
    }
}
