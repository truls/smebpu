#include <iostream>
#include <libsme/sme.h>

#include "memory_writer.h"
#include "constants.h"

void MemoryWriter::step() {

  // Store request in local buffer if we have room
  std::cout << "Buffer count " << buffer_fill_count << std::endl;
  if (wr_mem_valid_in->read() == 1 && buffer_fill_count != BUFFER_SIZE) {

    int buf_el = (buffer_start + buffer_fill_count) % BUFFER_SIZE;
    buffer[buf_el].adr = wr_mem_adr_in->read();
    buffer[buf_el].data = wr_data_in->read();
    buffer_fill_count++;
    std::cout << "Buffer: " << (unsigned)buf_el << " " << (unsigned)buffer[buf_el].adr << " " << (unsigned)buffer[buf_el].data << std::endl;
  }

  int ret = mem->fetch_write(pending_write, pending_reqid, 1);
  if (ret >= 0 && pending_write == 1) {
    pending_write = 0;
  }


  ret = mem->enqueue_write(pending_write == 0 && buffer_fill_count != 0, 0,
                           buffer[buffer_start].adr, buffer[buffer_start].data);
  std::cout << "Enqueued write " << (unsigned)buffer_start << " > " << (unsigned)buffer[buffer_start].data << std::endl;
  if (ret >= 0) {
    pending_reqid = ret;
    pending_write = 1;

    buffer_start = (buffer_start + 1) % BUFFER_SIZE;
    std::cout << "Reset buffer_start " << buffer_start  << std::endl;
    buffer_fill_count--;
  }

  wr_mem_available_out->write(BUFFER_SIZE - buffer_fill_count >= (PIPELINE_STAGES - 2) ? 1 : 0);
}
